#ifndef USER_H
#define USER_H

#include "stdafx.h"
#include "camera.h"
#include <vector>

class User
{
    public:
        /** Default constructor */
        User();
        /** Default destructor */
        virtual ~User();
        /** Access user_cameras
         * \return The current value of user_cameras
         */
        std::vector <camera> Getuser_cameras() { return user_cameras; }
        /** Set user_cameras
         * \param val New value to set
         */
        void Setuser_cameras(std::vector <camera> val) { user_cameras = val; }
        /** Access user_id;
         * \return The current value of user_id;
         */
        unsigned int Getuser_id() { return user_id; }
        /** Set user_id;
         * \param val New value to set
         */
        void Setuser_id(unsigned int val) { user_id = val; }
        /** Access user_name
         * \return The current value of user_name
         */
        std::string Getuser_name() { return user_name; }
        /** Set user_name
         * \param val New value to set
         */
        void Setuser_name(std::string val) { user_name = val; }
        /** Access user_pwd
         * \return The current value of user_pwd
         */
        std::string Getuser_pwd() { return user_pwd; }
        /** Set user_pwd
         * \param val New value to set
         */
        void Setuser_pwd(std::string val) { user_pwd = val; }

		//Maximum number of cameras per user

    protected:
    private:
        std::vector <camera> user_cameras; //!< Member variable "user_cameras"
        unsigned int user_id;; //!< Member variable "user_id;"
        std::string user_name; //!< Member variable "user_name"
        std::string user_pwd; //!< Member variable "user_pwd"
		
};

#endif // USER_H
