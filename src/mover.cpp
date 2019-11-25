// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// A tutorial on how to use the Gaze Interface.
//
// Author: Ugo Pattacini - <ugo.pattacini@iit.it>

#include <cstdlib>
#include <cmath>
#include <string>

#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/RpcClient.h>
#include <yarp/sig/Vector.h>
#include <yarp/math/Math.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;


class Mover: public RFModule
{
private:
    RpcClient port;
    bool init;
    Vector x0;
    int cnt;

public:
    virtual bool configure(ResourceFinder &rf)
    {
        port.open("/mover");
        if (!Network::connect(port.getName(),"/icubSim/world"))
        {
            yError()<<"Unable to connect to the world!";
            port.close();
            return false;
        }

        x0.resize(3);
        x0[0]=0.0;
        x0[1]=1.0;
        x0[2]=0.75;

        init=true;
        return true;
    }

    virtual bool close()
    {
		Bottle cmd,reply;
		cmd.addString("world");
		cmd.addString("del");
		cmd.addString("all");
		port.write(cmd,reply);

        port.close();
        return true;
    }

    virtual double getPeriod()
    {
        return 0.1;
    }

    virtual bool updateModule()
    {
        Bottle cmd,reply;

        if (init)
        {
            cmd.addString("world");
            cmd.addString("mk");
            cmd.addString("ssph");
            cmd.addDouble(0.03);
            cmd.addDouble(x0[0]);
            cmd.addDouble(x0[1]);
            cmd.addDouble(x0[2]);
            cmd.addDouble(1.0);
            cmd.addDouble(0.0);
            cmd.addDouble(0.0);
            cnt=0;
            init=false;
        }
        else
        {
            double t=(cnt++)*getPeriod();
            double phi=2.0*M_PI*(1.0/20.0)*t;
            
            Vector dx(3),x;
            dx[0]=cos(phi);
            dx[1]=sin(phi);
            dx[2]=sin(phi);
            x=x0+0.3*dx;

            cmd.addString("world");
            cmd.addString("set");
            cmd.addString("ssph");
            cmd.addInt(1);
            cmd.addDouble(x[0]);
            cmd.addDouble(x[1]);
            cmd.addDouble(x[2]);

            yInfo()<<"Target at "<<x.toString(3,3);
        }
        
        port.write(cmd,reply);
        return true;
    }
};



int main(int argc, char *argv[])
{
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError()<<"YARP doesn't seem to be available";
        return EXIT_FAILURE;
    }

    ResourceFinder rf;
    rf.configure(argc,argv);

    Mover mover;
    return mover.runModule(rf);
}
