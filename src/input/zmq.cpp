#include <atomic>
#include <zmq.hpp>
#include <iostream>

#include "input/zmq.hpp"

Generator<AER::Event> open_zmq(const std::string socket, std::atomic<bool>& runFlag) {
  zmq::context_t ctx;
  zmq::socket_t sock(ctx, zmq::socket_type::xsub);
  try {
    sock.set(zmq::sockopt::linger, 0);
    sock.connect(socket);
    auto header = std::string(&ZMQ_SUBSCRIBE_HEADER, 1);
    auto message = zmq::message_t{header.c_str(), header.size()};
    sock.send(message, zmq::send_flags::none);
  } catch (std::exception e) {
    std::cout << "Failed to connect to ZMQ socket " << socket << ": " << e.what() << std::endl;
  }

  zmq::pollitem_t items[] = {
    {sock.handle(), 0, ZMQ_POLLIN, 0},
  };
  zmq::message_t message;
  while (runFlag.load()) {
    zmq::poll(&items[0], sizeof(items) / sizeof(items[0]));
    if (!(items[0].revents & ZMQ_POLLIN)) { // No data to receive
      continue;
    } 

    auto rc = sock.recv(message);
    if (sock.get(zmq::sockopt::rcvmore) != 0) {
      const std::string header = message.to_string();
      if (header == "V") {
        auto rc2 = sock.recv(message);
        DvsEvent *data = message.data<DvsEvent>();
        co_yield {data->timestamp, data->x, data->y, data->polarity};
      }
    }
  }
  sock.close();
}