#ifndef CAMERA_H
#define CAMERA_H
#include <string>
#include "stdafx.h"

class camera
{
public:
    /** Default constructor */
    camera();

    /** Constructor by location */
    camera(std::string location);

    /** Constructor with all specifications */
    camera(std::string name, std::string model, std::string user, std::string pwd, std::string fps, std::string location, std::string resolution);

    void setName(std::string name);
    std::string getName();

    void setModel(std::string model);
    std::string getModel();

    void setUser(std::string user);
    std::string getUser();

    void setPwd(std::string password);
    std::string getPwd();

    void setFPS(std::string fps);
    std::string getFPS();

    void setLocation(std::string location);
    std::string getLocation();

    void setResolution(std::string resolution);
    std::string getResolution();

    bool isLocation(std::string location);

    /** Default destructor */
    virtual ~camera();
protected:
private:
    std::string cam_name;
    std::string cam_model;
    std::string cam_user;
    std::string cam_pwd;
    std::string cam_fps; //Unsure of whether most IP cameras can specify frames per second delivered
    std::string cam_location; //Format is http://ipcam:port/videostream.asf?user=user&pwd=pwd
    std::string cam_port;
    std::string cam_resolution; //Camera resolution by default is 320x240 (standard), otherwise can be 640x480
};

#endif // CAMERA_H
