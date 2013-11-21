#include "camera.h"
#include "stdafx.h"
camera::camera()
{
    cam_name = "Default Camera";
    cam_model = "Default Model";
    cam_user = "admin";
    cam_pwd = "";
    cam_resolution = "320x240";
    cam_fps = "6";
    cam_location = std::string("http://192.168.0.100") + std::string(":") + cam_port + std::string("/videostream.asf?user=") + cam_user + std::string("&pwd=") + cam_pwd + std::string("&resolution=") + cam_resolution + std::string("&rate=") + cam_fps;
}

camera::camera(std::string location)
{
    cam_name = "Default Camera";
    cam_model = "Default Model";
    cam_user = "admin";
    cam_pwd = "";
    cam_resolution = "320x240";
    cam_fps = "6";
    cam_location = location;

}

camera::camera(std::string name, std::string model, std::string user, std::string pwd, std::string fps, std::string location, std::string resolution)
{
    cam_name = name;
    cam_model = model;
    cam_user = user;
    cam_pwd = pwd;
    cam_fps = fps;
    cam_location = location;
    cam_resolution = resolution;
}

void camera::setName(std::string name)
{
    cam_name = name;
}

std::string camera::getName()
{
    return cam_name;
}

void camera::setModel(std::string model)
{
    cam_model = model;
}

void camera::setUser(std::string user)
{
    cam_user = user;
}

std::string camera::getUser()
{
    return cam_user;
}

void camera::setPwd(std::string pwd)
{
    cam_pwd = pwd;
}

std::string camera::getPwd()
{
    return cam_pwd;
}

void camera::setFPS(std::string fps)
{
    cam_fps = fps;
}

std::string camera::getFPS()
{
    return cam_fps;
}

void camera::setLocation(std::string location)
{
    cam_location = location;
}

std::string camera::getLocation()
{
    return cam_location;
}

void camera::setResolution(std::string resolution)
{
    cam_resolution = resolution;
}

std::string camera::getResolution()
{
    return cam_resolution;
}
camera::~camera()
{
    //dtor
}
