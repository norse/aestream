#include <atomic>
#include <csignal>
#include <stdexcept>
#include <string>
#include <sys/types.h>

#include "CLI11.hpp"

// AER imports
#include "aer.hpp"

// Input
#include "input/file.hpp"
#ifdef WITH_CAER
#include "input/inivation.hpp"
#endif
#ifdef WITH_METAVISION
#include "input/prophesee.hpp"
#endif

// Processing
#include "process/transformation.hpp"

// Output
#include "output/dvs_to_file.hpp"
#include "output/dvs_to_udp.hpp"

// Interrupt
auto runFlag = std::atomic<bool>(true);
void signalHandler(int signum) { runFlag.store(false); }


// Main
int main(int argc, char *argv[]) {

  signal(SIGINT, signalHandler); // Register interrupt

  CLI::App app{"Streams DVS data from a USB camera or AEDAT file to a file or "
               "UDP socket"};

  //
  // Input
  //
  auto app_input =
      app.add_subcommand("input", "Input source. Required")->required();

  // - DVS
  std::uint16_t deviceId;
  std::uint16_t deviceAddress;
  std::string camera = "davis";
  std::optional<std::string> serial_number = std::nullopt;
  // Inivation cameras
  auto app_input_inivation = app_input->add_subcommand(
      "inivation", "DVS input source for inivation cameras");
  app_input_inivation->add_option("id", deviceId, "Hardware ID");
  app_input_inivation->add_option("address", deviceAddress, "Hardware address");
  app_input_inivation->add_option("camera", camera,
                                  "Type of camera; davis or dvx");
  uint64_t timeLimitMs;
  app_input_inivation
      ->add_option("timeout", timeLimitMs,
                   "Time limit of the stream in ms. Defaults to infinity")
      ->default_val(-1);
  // Prophesee cameras
  auto app_input_prophesee = app_input->add_subcommand(
      "prophesee", "DVS input source for prophesee cameras");
  app_input_prophesee->add_option(
      "serial", serial_number,
      "Serial number (optional). Defaults to first available camera.");
  // - File
  std::string input_filename = "None";
  bool input_ignore_time = false;
  auto app_input_file = app_input->add_subcommand("file", "AEDAT4 input file");
  app_input_file
      ->add_option("file", input_filename, "Path to .aedat or .aedat4 file")
      ->required();
  // app_input_file->add_flag(
  //     "--ignore-time", input_ignore_time,
  //     "Playback in real-time (false, default) or ignore timestamps (true).");


  //
  // Processing
  //
  // Undistortion
  //
  auto app_undistortion = app.add_subcommand("undistortion", "Performs Undistortion");
  std::string undistortion_filename = "";
  app_undistortion->add_option("undistortion-filename", undistortion_filename,
                              "Undistortion Filename. Supports .csv");
  //
  // Transformation
  //
  auto app_transformation = app.add_subcommand("transformation", "Performs Transformation");
  std::string requested_trans = "";
  app_transformation->add_option("transformation-type", requested_trans,
                              "no_trans, rot_90, rot_180, rot_270, flip_ud, flip_lr");
 
  //
  // Spatio-Temporal Sampling
  //
  auto app_sampling = app.add_subcommand("sampling", "Spatio-Temporal Sampling");
  std::uint8_t t_sample = 1;
  std::uint8_t s_sample = 1;
  app_sampling->add_option("t", t_sample, "1 out of <t> events are sent");
  app_sampling->add_option("s", s_sample, "<s>x<s> pixels become 1 pixel");

  //
  // Resolution
  //
  auto app_resolution = app.add_subcommand("resolution", "Provides resolution");
  std::uint16_t width;
  std::uint16_t height;
  app_resolution->add_option("w", width, "Camera width (in pixels)");
  app_resolution->add_option("h", height, "Camera height (in pixels)");


  //
  // Output
  //
  auto app_output =
      app.add_subcommand("output", "Output target. Defaults to stdout");
  // - STDOUT
  auto app_output_stdout = app_output->add_subcommand("stdout");
  // - UDP
  std::string port = "3333";           // Port number
  std::string ipAddress = "localhost"; // IP Adress - if NULL, use own IP.
  std::uint32_t bufferSize = 1024;
  std::uint16_t packetSize = 128;
  bool include_timestamp = false;
  auto app_output_udp = app_output->add_subcommand(
      "udp", "SpiNNaker Interface Board (SPIF) output over UDP");
  app_output_udp->add_option("destination", ipAddress,
                             "Destination IP. Defaults to localhost");
  app_output_udp->add_option("port", port,
                             "Destination port. Defaults to 3333");
  app_output_udp->add_option("--buffer-size", bufferSize,
                             "UDP buffer size. Defaults to 1024");
  app_output_udp->add_option(
      "--packet-size", packetSize,
      "Number of events in a single UDP packet. Defaults to 128");
  app_output_udp->add_option("--include-timestamp", include_timestamp,
                             "Include timestamp in events");
  // - FILE
  std::string output_filename;
  auto app_output_file = app_output->add_subcommand("file", "File output");
  app_output_file->add_option("output-filename", output_filename,
                              "Output Filename. Supports .txt or .aedat4");

  //
  // Generate options
  //
  std::int64_t maxPackets = -1;
  app.add_option("--max-packets", maxPackets,
                 "Maximum number of packets to read before stopping. Defaults "
                 "to -1 (infinite).");

  CLI11_PARSE(app, argc, argv);

  //
  // Handle input
  //
  Generator<AER::Event> input_generator;
  std::unique_ptr<FileBase> file_handle = nullptr;
  if (app_input_inivation->parsed()) {
#ifdef WITH_CAER
    input_generator = inivation_event_generator(
        deviceId > 0 ? std::make_optional(InivationDeviceAddress{
                           camera, deviceId, deviceAddress})
                     : std::nullopt,
        runFlag);
#else
    throw std::invalid_argument(
        "Inivation cameras unavailable: please recompile with libcaer");
#endif
  } else if (app_input_prophesee->parsed()) {
#ifdef WITH_METAVISION
    input_generator = prophesee_event_generator(runFlag, serial_number);
#else
    throw std::invalid_argument(
        "Prophesee cameras unavailable: please recompile with MetavisionSDK");
#endif
  } else if (app_input_file->parsed()) {
    file_handle = open_event_file(input_filename);
    input_generator = file_handle->stream();
  }

  //
  // Processing (Undistort, Mirror, Flip, Turn)
  //
  bool processing = false;
  if(undistortion_filename.length() > 0){
    processing = true;
  }
  
  Generator<AEDAT::PolarityEvent> processed_generator;
  Generator<AEDAT::PolarityEvent> * ptr_processed_generator;
  if(processing){
    processed_generator = undistortion_event_generator(input_generator, undistortion_filename, width, height);
    ptr_processed_generator = &processed_generator;
  } else {    
    ptr_processed_generator = &input_generator;
  }

  //
  // Handle output
  //
  try {
    if (app_output_udp->parsed()) {
      std::cout << "Sending events to: " << ipAddress << " on port: " << port
                << std::endl;
      DVSToUDP<AER::Event> client(bufferSize, port, ipAddress);
      client.stream(*ptr_processed_generator, include_timestamp);
    } else if (app_output_file->parsed()) {
      std::cout << "Sending events to file " << output_filename << std::endl;
      if (output_filename.ends_with(".txt")) {
        dvs_to_file_txt(*ptr_processed_generator, output_filename);
      } else if (output_filename.ends_with(".aedat4")) {
        dvs_to_file_aedat(*ptr_processed_generator, output_filename);
      } else {
        std::stringstream error;
        error << "Unsupported file ending" << output_filename;
        throw std::invalid_argument(error.str());
      }
    } else { // Default to STDOUT
      uint64_t count = 0;
      for (AER::Event event : *ptr_processed_generator) {
        count += 1;
        std::cout << std::to_string(event.timestamp) << ","
                  << event.x << "," << event.y
                  << std::endl;
      }
      std::cout << "Sent a total of " << count << " events" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cout << "Failure while streaming events: " << e.what() << "\n";
  }
}