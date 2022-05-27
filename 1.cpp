#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <errno.h>
#include <deque>
#include <cstdlib>
#include "bitmap_image.hpp"
using namespace std;
int screen_width,screen_height;
double x_right_limit,x_left_limit;
double y_top_limit,y_bottom_limit;
double z_front_limit,z_rear_limit;
double dx,dy,Top_y,Left_x;
double ** Z_buffer;
int *** frame_buffer;
struct Point{
    double x,y,z;
};
struct Triangle{
    Point points[3];
    int color[3];
};
deque <struct Triangle> objects;
void read_data()
{
    string path1,path2;
    string line;
    double x,y,z;
    int line_count = 0;
    path1 = "config.txt";
    path2 = "stage3.txt";
    ifstream infile1(path1.c_str());
    while(line_count < 4){
        std::getline(infile1, line);
        std::istringstream iss(line);
        switch(line_count)
        {
            case 0:
                iss>>x>>y;
                screen_height = y;
                screen_width = x;
                break;
            case 1:
                iss >>x;
                x_left_limit = x;
                x_right_limit = -x;
                break;
            case 2:
                iss>>y;
                y_bottom_limit = y;
                y_top_limit = -y;
                break;
            case 3:
                iss>>x>>y;
                z_front_limit = x;
                z_rear_limit = y;
                break;
            default:
                break;
        }
        line_count++;
    }
    line_count = 0;
    ifstream infile2(path2.c_str());
    struct Triangle t;
    while(std::getline(infile2,line))
    {
        if((line_count+1)%4==3){
            struct Point p;
            std::istringstream iss(line);
            iss>>x>>y>>z;
            int idx = ((line_count+1)%4)-1;
            //p.x = x;
            //p.y = y;
            //p.z = z;
            //cout<<p.x<<";"<<p.y<<";"<<p.z<<endl;
            //cout<<idx<<endl;
            //cout<<x<<y<<z<<endl;
            t.points[idx].x = x;
            t.points[idx].y = y;
            t.points[idx].z = z;
            t.color[idx] = rand() % 256;
            //cout<<t.color[line_count%4]<<endl;
            //cout<<"Last Line"<<endl;
        }
        else if((line_count+1)%4==0){
            //cout<<"New Line"<<endl;
            //cout<<t.points[0].y<<t.points[1].y<<t.points[2].y<<endl;
            objects.push_back(t);
            struct Triangle t;
            x=0;y=0;z=0;
        }
        else{
            struct Point p;
            std::istringstream iss(line);
            iss>>x>>y>>z;
            //p.x = x;
            //p.y = y;
            //p.z = z;
            int idx = ((line_count+1)%4)-1;
            //cout<<idx<<endl;
            //cout<<p.x<<";"<<p.y<<";"<<p.z<<endl;
            //cout<<x<<y<<z<<endl;
            t.points[idx].x = x;
            t.points[idx].y = y;
            t.points[idx].z = z;
            t.color[idx] = rand() % 256;
            //cout<<t.color[line_count%4]<<endl;
        }
        line_count++;
    }
}
void initialize_z_buffer_and_frame_buffer()
{
    double diffx,diffy;
    diffx = (x_right_limit - x_left_limit);
    diffy = (y_top_limit - y_bottom_limit);
    dx =  diffx/screen_width;
    dy =  diffy/screen_height;
    //cout<<dx<<" "<<dy<<endl;
    Top_y = y_top_limit - (dy / 2);
    Left_x = x_left_limit + (dx / 2);
    //cout<<Top_y<<" "<<Left_x<<endl;
    //now convert to pixel
    //convert_to_pixel();
    Z_buffer = new double*[screen_width];
    frame_buffer = new int**[screen_width];
    for(int i = 0; i < screen_width; i++){
        Z_buffer[i] = new double[screen_height];
        frame_buffer[i] = new int*[screen_height];
        for(int j = 0; j < screen_height; j++){
            frame_buffer[i][j] = new int[3];
        }
    }
    for(int i = 0; i < screen_width; i++){
        for(int j = 0; j < screen_height; j++){
            Z_buffer[i][j] = z_rear_limit - z_front_limit;
            for(int k = 0; k <3 ; k++){
                frame_buffer[i][j][k] = 0;
            }
        }
    }
}
struct Triangle clip_y(struct Triangle t)
{
    for(int i=0;i<3;i++){
        Point p;
        p = t.points[i];
        if(p.y>y_top_limit){
            t.points[i].y = y_top_limit;
            //cout<<t.points[i].y<<"inside"<<endl;
        }
        else if(p.y<y_bottom_limit){
            t.points[i].y = y_bottom_limit;
            //cout<<t.points[i].y<<"inside"<<endl;
        }
        else{
            //do nothing
        }
    }
    return t;
}
struct Triangle clip_x(struct Triangle t)
{
    for(int i = 0; i < 3; i++){
        Point p;
        p = t.points[i];
        if(p.x>x_right_limit){
            t.points[i].x = x_right_limit;
            //cout<<t.points[i].y<<"inside"<<endl;
        }
        else if(p.x<x_left_limit){
            t.points[i].x = x_left_limit;
            //cout<<t.points[i].y<<"inside"<<endl;
        }
        else{
            //do nothing
        }
    }
    return t;
};
struct Triangle convert_to_pixel_row(struct Triangle t)
{
    for(int i=0;i<3;i++){
        t.pp[i].py = (int)((Top_y - t.points[i].y)/dy);
        //cout<<t.pp[i].py<<" pixely"<<endl;
    }
    return t;
}
struct Triangle convert_to_pixel_col(struct Triangle t)
{
    for(int i=0;i<3;i++){
        t.pp[i].px = (int)((t.points[i].x-Left_x)/dx);
        //cout<<t.pp[i].py<<" pixely"<<endl;
    }
    return t;
};
struct Triangle sort_on_y(struct Triangle t)
{
    struct Point p1,p2,p3;
    int top_idx,bottom_idx,other_idx,i1,i2;
    //sort the top
    if(t.points[0].y > t.points[1].y){
        if(t.points[0].y > t.points[2].y){
            p1 = t.points[0];top_idx = 0;
        }
        else{
            p1 = t.points[2];top_idx = 2;
        }
    }
    else{
        if(t.points[1].y > t.points[2].y){
            p1 = t.points[1];top_idx = 1;
        }
        else{
            p1 = t.points[2];top_idx = 2;
        }
    }
    //search for bottom
    if(top_idx == 0){
        i1 = 1; i2 = 2;
    }
    else if(top_idx == 1){
        i1 = 0; i2 = 2;
    }
    else{
        i1 = 0; i2 = 1;
    }
    if(t.points[i1].y < t.points[i2].y){
        p3 = t.points[i1]; p2 = t.points[i2];
        bottom_idx = i1;other_idx = i2;
    }
    else{
        p3 = t.points[i2]; p2 = t.points[i1];
        bottom_idx = i2; other_idx = i1;
    }
    t.points[0] = p1;
    //cout<<t.points[0].x<<","<<t.points[0].y<<","<<t.points[0].z<<endl;
    t.points[1] = p2;
    //cout<<t.points[1].x<<","<<t.points[1].y<<","<<t.points[1].z<<endl;
    t.points[2] = p3;
    //cout<<t.points[2].x<<","<<t.points[2].y<<","<<t.points[2].z<<endl;
    return t;
};
void apply_procedure()
{
    double x1,x2,z1,z2,z;
    ofstream ofile;
    ofile.open("z_buffer.txt");
    for(int i=0; i< objects.size(); i++){
        ofile<<"Triangle "<<i<<'\n';
        struct Triangle t;
        t = objects[i];
        double upper_y = max(max(t.points[0].y,t.points[1].y),t.points[2].y);
        double lower_y = min(min(t.points[0].y,t.points[1].y),t.points[2].y);
        double upper_intersecting_point = max(min(upper_y,y_top_limit),y_bottom_limit);
        double lower_intersecting_point = min(max(lower_y,y_bottom_limit),y_top_limit);
        int top_scan_line = (int)((Top_y - upper_intersecting_point)/dy);
        int bottom_scan_line = (int)((Top_y - lower_intersecting_point)/dy);
        //cout<<upper_y<<","<<lower_y<<endl;
        //cout<<upper_intersecting_point<<";"<<lower_intersecting_point<<endl;
        //ofile<<top_scan_line<<";"<<bottom_scan_line<<'\n';
        t = sort_on_y(objects[i]);
        //cout<<"sorted"<<endl;
        //ofile<<t.points[0].x<<";"<<t.points[0].y<<";"<<t.points[0].z<<'\n';
        //ofile<<t.points[1].x<<";"<<t.points[1].y<<";"<<t.points[1].z<<'\n';
        //ofile<<t.points[2].x<<";"<<t.points[2].y<<";"<<t.points[2].z<<'\n';
        for(int row = top_scan_line; row < bottom_scan_line; row++){
            //(p1,p2) and (p1,p3) until row = p2's y pixel value
            // when we hit row = p2's pixel, we throw out (p1,p2) and introduce (p2,p3)
            double y = Top_y - (row * dy);
            //ofile<<"ys "<<y;
            if(y>t.points[1].y){
                x1 = t.points[0].x + ((y-t.points[0].y)* (t.points[1].x - t.points[0].x)/(t.points[1].y-t.points[0].y));
                z1 = t.points[0].z + ((y-t.points[0].y)*(t.points[1].z - t.points[0].z)/(t.points[1].y-t.points[0].y));
            }
            else{
                x1 = t.points[1].x + ((y-t.points[1].y)*(t.points[2].x - t.points[1].x)/(t.points[2].y-t.points[1].y));
                z1 = t.points[1].z + ((y-t.points[1].y)*(t.points[2].z - t.points[1].z)/(t.points[2].y-t.points[1].y));
            }
            x2 = t.points[0].x + ((y-t.points[0].y)*(t.points[2].x - t.points[0].x)/(t.points[2].y-t.points[0].y));
            z2 = t.points[0].z + ((y-t.points[0].y)*(t.points[2].z - t.points[0].z)/(t.points[2].y-t.points[0].y));
            //cout<<"x1 "<<x1<<" x2 "<<x2<<endl;
            //ofile<<" x1,z1 "<<x1<<' '<<z1<<" x2,z2 "<<x2<<' '<<z2<<'\n';
            //cout<<"z1 "<<z1<<" z2 "<<z2<<endl;
            double left_intersecting_point,leftz,leftx;
            double right_intersecting_point,rightz,rightx;
            if(x1<x2){
                leftx = x1;
                rightx = x2;
                leftz = z1;
                rightz = z2;
            }
            else{
                leftx = x2;
                rightx = x1;
                leftz = z2;
                rightz = z1;
            }
            //ofile<<" leftx,leftz "<<leftx<<' '<<leftz<<" rightx,rightz "<<rightx<<' '<<rightz<<'\n';
            left_intersecting_point = min(max(leftx,x_left_limit),x_right_limit);
            right_intersecting_point = max(min(rightx,x_right_limit),x_left_limit);
            int left_intersecting_col = (int)((left_intersecting_point-Left_x)/dx);
            int right_intersecting_col = (int)((right_intersecting_point-Left_x)/dx);
            //ofile<<" left_intersec, right_intersec "<<left_intersecting_point<<","<<right_intersecting_point<<'\n';
            //ofile<<left_intersecting_col<<";"<<right_intersecting_col<<'\n';
            //cout<<right_z<<";"<<left_z<<endl;
            //cout<<"x1 "<<x1<<"left "<<left_intersecting_point<<" right"<<right_intersecting_point <<"x2 "<<x2<<endl;
            for(int col=left_intersecting_col; col < right_intersecting_col; col++){
                double x = Left_x + (col * dx);
                double Z = leftz + ((x-leftx)*(rightz-leftz)/(rightx - leftx));
                //cout<<Z<<endl;
                //ofile<<" Z value "<<Z<<'\n';
                if(Z<0){
                    //cout<<Z<<endl;
                    Z = 0;
                }
                if(Z<Z_buffer[row][col]){
                    Z_buffer[row][col] = Z;
                    //cout<<Z_buffer[row][col]<<endl;
                    frame_buffer[row][col][0] = t.color[0];
                    frame_buffer[row][col][1] = t.color[1];
                    frame_buffer[row][col][2] = t.color[2];
                }
            }
        }
    }
    double zmax = z_rear_limit - z_front_limit;
    for(int i = 0; i < screen_width; i++){
        for(int j = 0; j < screen_height; j++){
            if(Z_buffer[i][j]<zmax){
                ofile<<Z_buffer[i][j]<<" ";
            }
        }
        ofile<<'\n';
    }
    ofile.close();
}
void draw_image(string name)
{
    bitmap_image image(screen_width,screen_height);//(width,height)

    for(int i=0;i<screen_width;i++){
        for(int j=0;j<screen_height;j++){
            image.set_pixel(j,i,frame_buffer[i][j][0],frame_buffer[i][j][1],frame_buffer[i][j][2]);
        }
    }
    image.save_image(name);
}
void free_memory()
{
    delete Z_buffer;
    delete frame_buffer;
}
void zbuffer()
{
    read_data();
    initialize_z_buffer_and_frame_buffer();
    apply_procedure();
    draw_image("1.bmp");
    free_memory();
}
int main()
{
    zbuffer();
    return 0;
}
