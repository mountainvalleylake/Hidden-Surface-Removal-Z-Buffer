#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <errno.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <iterator>
#include <cstdlib>
#include <deque>
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
struct Plane{
    double A,B,C,D;
};
struct Polygon{
    struct Plane Pl;
    int color[3];
    bool in_out;
    int serial_no;
};
struct Triangle{
    Point points[3];
    int color[3];
};
struct Edge{
    double x_min;
    double y_min;
    double y_max;
    double ddx;
    int polygon_id;
};
struct ActiveEdge{
    //double x_max;
    double y_max;
    double x_a;
    double ddx;
    int polygon_id;
};
vector <struct Triangle> objects;
vector <struct Polygon> polygon_table;
map <int,vector <struct Edge> > edge_table;
vector <struct ActiveEdge> active_edge_table;
vector <struct Polygon> active_polygon_table;
vector<struct ActiveEdge>::iterator pos;
vector<struct Polygon>::iterator position;
bool double_equals(double a, double b, double epsilon)
{
    return std::abs(a - b) < epsilon;
}
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
void remove_polygon(std::vector<struct Polygon> & pets, int idx) {
    pets.erase(
        std::remove_if(pets.begin(), pets.end(), [&](Polygon const & pet) {
            return pet.serial_no == idx;
        }),
        pets.end());
}
void remove_edge(std::vector<struct ActiveEdge> & e, double y, double dy){
    e.erase(
            std::remove_if(e.begin(),e.end(), [&](ActiveEdge const &e){
                           return double_equals(e.y_max,y,dy);}),e.end());
}
void apply_procedure()
{
    int init = edge_table.begin()->first;
    int fin = screen_height;
    double y;
    y = Top_y - init * dy;
    cout<<y<<endl;
    //cout<<edge_table.size()<<endl;
    while(init>=0){
        // find the keys that fall within the upper bound and the lower bound
        //if(y < -0.2)cout<<y<<endl;
        vector<struct Edge> edge_value;
        map<int, vector <struct Edge> >::iterator it;
        it = edge_table.find(init);
        //cout<<it->first<<endl;
        if(it != edge_table.end()){
            //cout<<"first"<<endl;
            //found the key
            //cout<<"Found key"<<endl;
            edge_value = it->second;
            //cout<<y<<init<<endl;
            for(int k=0; k<edge_value.size();k++){
                //cout<<"second"<<endl;
                //struct Edge ed;
                //ed = edge_value[k];
                struct ActiveEdge aed;
                //aed.x_max = edge_value[k].x_max;
                aed.y_max = edge_value[k].y_max;
                aed.ddx = edge_value[k].ddx;
                aed.x_a = edge_value[k].x_min;
                aed.polygon_id = edge_value[k].polygon_id;
                active_edge_table.push_back(aed);
                cout<<"Active edge table size : "<<active_edge_table.size()<<endl;
            }
        }
        sort(active_edge_table.begin(),active_edge_table.end(),[]
        (const struct ActiveEdge &lhs,const struct ActiveEdge &rhs)
        {return lhs.x_a < rhs.x_a;});
        double first_x,next_x;
        for(int k=0; k< active_edge_table.size()-1;k++){
            //cout<<"fifth"<<endl;
            //struct ActiveEdge ae;
            //ae = active_edge_table[k];
            int polygon_idx = active_edge_table[k].polygon_id;
            bool status = polygon_table[polygon_idx].in_out;
            if(status==0){
                polygon_table[polygon_idx].in_out = 1;
                struct Polygon p;
                p.in_out = polygon_table[polygon_idx].in_out;
                p.color[0] = polygon_table[polygon_idx].color[0];
                p.color[1] = polygon_table[polygon_idx].color[1];
                p.color[2] = polygon_table[polygon_idx].color[2];
                p.serial_no = polygon_idx;
                p.Pl.A = polygon_table[polygon_idx].Pl.A;
                p.Pl.B = polygon_table[polygon_idx].Pl.B;
                p.Pl.C = polygon_table[polygon_idx].Pl.C;
                p.Pl.D = polygon_table[polygon_idx].Pl.D;
                    //p.serial_no = polygon_table[polygon_idx].serial_no;
                active_polygon_table.push_back(p);
                //cout<<"fifth1"<<endl;
                //cout<<"Here "<<active_polygon_table.size()<<endl;
        }
        else{
            struct Polygon p;
            p = polygon_table[polygon_idx];
            remove_polygon(active_polygon_table,polygon_idx);
            polygon_table[polygon_idx].in_out = 0;
                //cout<<"Now "<<active_polygon_table.size()<<endl;
                //cout<<"fifth2"<<endl;
        }
        cout<<"start x "<<active_edge_table[k].x_a<<endl;
        //cout<<"inc x "<<active_edge_table[k].ddx <<endl;
        first_x = min(max(active_edge_table[k].x_a,x_left_limit),x_right_limit);
        next_x = min(active_edge_table[k+1].x_a,x_right_limit);
        double x= first_x;
        while(x<=next_x){
                //cout<<"after fifth"<<endl;
            double z;
            if(polygon_table[polygon_idx].Pl.C != 0){
                z = -(polygon_table[polygon_idx].Pl.A*x + polygon_table[polygon_idx].Pl.B*y + polygon_table[polygon_idx].Pl.D)/polygon_table[polygon_idx].Pl.C;
                if(z < 0){
                    z = 0;
                }
                //cout<<x<<' '<<y<<' '<<z<<endl;
                int row,col;
                row = init;
                col = (int) ((x - Left_x)/dx);
                    //cout<<"dx "<<dx<<endl;
                if(z < Z_buffer[row][col]){
                    //cout<<"row,col,z "<<row<<' '<<col<<' '<<z<<endl;
                    Z_buffer[row][col] = z;
                    //if(polygon_idx != 0) cout<<"Polygon Index "<<polygon_idx<<endl;
                    frame_buffer[row][col][0] = polygon_table[polygon_idx].color[0];
                    frame_buffer[row][col][1] = polygon_table[polygon_idx].color[1];
                    frame_buffer[row][col][2] = polygon_table[polygon_idx].color[2];
                }
            }
                //cout<<"z "<<z<<' '<<pol.Pl.A<<pol.Pl.B<<pol.Pl.C<<pol.Pl.D<<endl;
                //cout<<x<<' '<<y<<' '<<z<<endl;
                //x += active_edge_table[k].ddx;
            x += dx;
                //cout<<"new x "<<x<<endl;
        }
        remove_edge(active_edge_table,y,dy);
        cout<<"y "<<active_edge_table.size()<<endl;
        //for(int k=0; k< active_edge_table.size();k++){
            //cout<<"ymax "<<active_edge_table[k].y_max<<endl;
        //}
        //cout<<" now active edge table size : "<<active_edge_table.size()<<endl;
        for(int k=0; k< active_edge_table.size();k++){
                //cout<<"sixth"<<endl;
            active_edge_table[k].x_a = active_edge_table[k].x_a + (active_edge_table[k].ddx*dy);
                //cout<<"Update "<<active_edge_table[k].x_a<<endl;
        }
        sort(active_edge_table.begin(),active_edge_table.end(),[]
        (const struct ActiveEdge &lhs,const struct ActiveEdge &rhs)
        {return lhs.x_a < rhs.x_a;});}
        init -= 1;
        y += dy;
    }
}



void initialize_edge_table_and_polygon_table()
{
    double diffx,diffy;
    diffx = (x_right_limit - x_left_limit);
    diffy = (y_top_limit - y_bottom_limit);
    dx =  diffx/screen_width;
    dy =  diffy/screen_height;
    Top_y = y_top_limit - (dy / 2);
    Left_x = x_left_limit + (dx / 2);
    for(int i=0; i<objects.size();i++){
        struct Triangle t;
        t = objects[i];
        struct Point vector1;
        vector1.x = t.points[1].x - t.points[0].x;
        vector1.y = t.points[1].y - t.points[0].y;
        vector1.z = t.points[1].z - t.points[0].z;
        struct Point vector2;
        vector2.x = t.points[2].x - t.points[0].x;
        vector2.y = t.points[2].y - t.points[0].y;
        vector2.z = t.points[2].z - t.points[0].z;
        struct Point normal;
        normal.x = vector1.y * vector2.z - vector2.y * vector1.z;
        normal.y = vector1.x * vector2.z - vector2.x * vector1.z;
        normal.z = vector1.x * vector2.y - vector2.x * vector1.z;
        double d = -(normal.x * t.points[0].x + normal.y * t.points[0].y + normal.z * t.points[0].z);
        struct Polygon pol;
        struct Plane planes;
        pol.Pl.A = normal.x; pol.Pl.B = normal.y; pol.Pl.C = normal.z; pol.Pl.D = d;
        //cout<<"planes "<<pol.Pl.A<<' '<<pol.Pl.B<<' '<<pol.Pl.C<<' '<<pol.Pl.D;
        //pol.Pl = planes;
        pol.color[0] = t.color[0]; pol.color[1] = t.color[1]; pol.color[2] = t.color[2];
        pol.in_out = false;
        pol.serial_no = i;
        polygon_table.push_back(pol);
        for(int j=0; j<3; j++){
            struct Point p1;
            struct Point p2;
            double ymax,ymin,xmin,m,delx;
            p1 = t.points[j];
            p2 = t.points[(j+1)%3];
            //cout<<p1.x<<" "<<p1.y<<" "<<p1.z<<endl;
            //cout<<p2.x<<" "<<p2.y<<" "<<p2.z<<endl;
            if(p1.y > p2.y){
                ymax = p1.y;ymin = p2.y;xmin = p2.x;
                m = (p1.y-p2.y)/(p1.x-p2.x);delx = 1/m;
            }
            else{
                ymax = p2.y;ymin = p1.y;xmin = p1.x;
                m = (p2.y-p1.y)/(p2.x-p1.x);delx = 1/m;
            }
            if(!double_equals(m,0,0.001)){
                struct Edge e;
                cout<<"correct edge "<<endl;
                e.x_min = xmin; e.y_min = ymin; e.y_max = ymax;
                e.ddx = delx; e.polygon_id = i;
                //cout<<m<<e.ddx<<e.polygon_id<<e.x_min<<e.y_min<<e.y_max<<endl;
                double miny;
                if(e.y_min < y_bottom_limit){
                    miny = y_bottom_limit;
                }
                else if(e.y_min > y_top_limit){
                    miny = y_top_limit;
                }
                else{
                    miny = e.y_min;
                }
                int row;
                row = (int) ((Top_y - miny)/dy);
                //cout<<"dy "<<dy<<endl;
                //cout<<"row "<<row<<endl;
                //miny = Top_y - row * dy;
                //cout<<"miny "<<miny<<endl;
                map<int, vector <struct Edge> >::iterator it;
                //it = edge_table.find(y);
                it = edge_table.find(row);
                if(it != edge_table.end()){
                    //found the key
                    vector<struct Edge> edgelist;
                    edgelist = it->second;
                    edgelist.push_back(e);
                    //sort after insert
                    cout<<"Duplicate"<<endl;
                    sort(edgelist.begin(),edgelist.end(),[]
                         (const struct Edge &lhs,const struct Edge &rhs)
                         {return lhs.x_min < rhs.x_min;});
                    it->second = edgelist;
                }
                else{
                    vector<struct Edge> edgelist;
                    edgelist.push_back(e);
                    edge_table.insert(std::pair<int,vector<struct Edge> >(row,edgelist));
                }
            }
        }
    }
    map<int, vector <struct Edge> >::iterator it;
    for (it=edge_table.begin(); it!=edge_table.end(); ++it)
        std::cout << it->first << " => " << it->second.size()<< '\n';
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
void free_memory()
{
    //delete edge_table;
    //delete polygon_table;
    delete Z_buffer;
    delete frame_buffer;
}
void scan_line()
{
    read_data();
    initialize_edge_table_and_polygon_table();
    apply_procedure();
    draw_image("2.bmp");
    free_memory();
}
int main()
{
    scan_line();
    return 0;
}
