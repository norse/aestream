
#include "./transformation.hpp"
using namespace std;



void get_new_lut(int width, int height, map lut[], bool empty){    

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {   
            if(empty){         
                lut[x*height+y].np = 0;
                lut[x*height+y].p[0].x = -1;            
                lut[x*height+y].p[0].y = -1;   
            } else {
                lut[x*height+y].np = 1;
                lut[x*height+y].p[0].x = x;            
                lut[x*height+y].p[0].y = y;  
            }       
            lut[x*height+y].p[1].x = -1;      
            lut[x*height+y].p[1].y = -1;
        }
    }
}
void load_undistortion_lut(const std::string & fname, int width, int height, map lut[]){

    
    // File pointer
    std::ifstream infile(fname);  

    vector<string> row;
    string line, val, temp;  

    while (std::getline(infile, line))
    {
        stringstream s(line);
  
        int tok_ix = 0;
        int np = 0;
        int lutix;
        bool done = false;
        while (getline(s, val, ',')) 
        {
            switch(tok_ix) {
                case 0:
                    lutix = stoi(val);
                    break;
                case 1:
                    lut[lutix].p[0].x= stoi(val);
                    if (lut[lutix].p[0].x >= 0){
                        np += 1;
                    }
                    break;
                case 2:
                    lut[lutix].p[0].y= stoi(val);
                    lut[lutix].np= np;
                    break;
                case 3:
                    lut[lutix].p[1].x= stoi(val);
                    if (lut[lutix].p[1].x >= 0){
                        np += 1;
                    }
                    break;
                case 4:
                    lut[lutix].p[1].y= stoi(val);
                    lut[lutix].np= np;
                    break;
                case 5:
                    done = true;
                    break;
            }
            tok_ix += 1;
        }
    }
    
    
}


  trans from_string_to_trans(std::string requested_trans){

    trans transformation = no_trans;

    if(requested_trans.compare("no_trans") == 0){
        transformation = no_trans;
    }
    if(requested_trans.compare("rot_90") == 0){
        transformation = rot_90;
    }
    if(requested_trans.compare("rot_180") == 0){
        transformation = rot_180;
    }
    if(requested_trans.compare("rot_270") == 0){
        transformation = rot_270;
    }
    if(requested_trans.compare("flip_ud") == 0){
        transformation = flip_ud;
    }
    if(requested_trans.compare("flip_lr") == 0){
        transformation = flip_lr;
    }

    return transformation;

  }

void update_lut_element(map lut[], map aux[], int ix_lut, int ix_aux, bool cross, uint8_t s_sample){  
    for (int pixix = 0; pixix < MAX_PIXIX; pixix++) {
        aux[ix_aux].np = lut[ix_lut].np;
        if(cross){
            aux[ix_aux].p[pixix].x = lut[ix_lut].p[pixix].y/s_sample;            
            aux[ix_aux].p[pixix].y = lut[ix_lut].p[pixix].x/s_sample; 
        } else {
            aux[ix_aux].p[pixix].x = lut[ix_lut].p[pixix].x/s_sample;            
            aux[ix_aux].p[pixix].y = lut[ix_lut].p[pixix].y/s_sample;  
        }
    }
}



void trans_lut(std::uint16_t * i_width, std::uint16_t * i_height, map lut[], map aux[], trans trans_type, uint8_t s_sample){

    uint16_t o_width = *i_width;
    uint16_t o_height= *i_height;

    uint16_t x, y, new_x, new_y, new_h;
    bool cross;
    for(uint64_t i = 0; i < o_width*o_height; i++) {
        x = i/o_height;
        y = i%o_height;
        if(x%s_sample==0 && y%s_sample==0){
            switch(trans_type) {
                case rot_90:
                    new_x = o_width-1-x;
                    new_y = y;
                    new_h = o_width;
                    cross = true;
                    break;
                case rot_180:
                    new_x = o_width-1-x;
                    new_y = o_height-1-y;
                    new_h = o_height;
                    cross = false;
                    break;
                case rot_270:
                    new_x = x;
                    new_y = o_height-1-y;
                    new_h = o_width;
                    cross = true;
                    break;
                case flip_lr:
                    new_x = o_width-1-x;
                    new_y = y;
                    new_h = o_height;
                    cross = false;
                    break;
                case flip_ud:
                    new_x = x;
                    new_y = o_height-1-y;
                    new_h = o_height;
                    cross = false;
                    break;
                default:
                    new_x = x;
                    new_y = y;
                    new_h = o_height;
                    cross = false;
                    break;

            }
            update_lut_element(lut, aux, x*o_height+y, (new_x*new_h+new_y), cross, s_sample);
        }
    }

    if (cross){
        *i_width = o_height;
        *i_height = o_width;
    }

    memcpy(lut, aux, MAX_W*MAX_H*sizeof(map)); 
    
}



Generator<AEDAT::PolarityEvent>
transformation_event_generator(Generator<AEDAT::PolarityEvent> &input_generator,
                       const std::string &undistortion_filename, trans transformation, 
                       std::uint16_t width, std::uint16_t height, uint8_t t_sample, uint8_t s_sample) {
 
    map lut[MAX_W*MAX_H];
    map aux[MAX_W*MAX_H];

    bool empty;
 
    if(undistortion_filename.length() > 0){
        empty = true; 
        get_new_lut(width, height, aux, empty);  
        load_undistortion_lut(undistortion_filename, width, height, lut);
    } else {
        empty = false; 
        get_new_lut(width, height, lut, empty);  
    }
    trans_lut(&width, &height, lut, aux, transformation, s_sample);

    uint16_t new_x;
    uint16_t new_y;
    uint64_t count;

    for (auto event : input_generator) {
       
        for(int pixix = 0; pixix < lut[event.x*height+event.y].np; pixix++){
            new_x = lut[event.x*height+event.y].p[pixix].x;
            new_y = lut[event.x*height+event.y].p[pixix].y;
            event.x = new_x;
            event.y = new_y;
            if(count%t_sample == 0){
                co_yield event;
            }
            count+= 1;
        }
    }        

}