// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// A tutorial on how to use the Gaze Interface.
//
// Author: Ugo Pattacini - <ugo.pattacini@iit.it>

#include <string>

#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>

#include <yarp/dev/Drivers.h>
#include <yarp/dev/GazeControl.h>
#include <yarp/dev/PolyDriver.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;


class CtrlThread: public RateThread
{
protected:
    PolyDriver     clientGaze;
    IGazeControl  *igaze;

    int state;
    int startup_context_id;

    BufferedPort<Bottle> port;

public:
    CtrlThread() : RateThread(1000) { }

    virtual bool threadInit()
    {
        // open a client interface to connect to the gaze server
        // we suppose that:
        // 1 - the iCub simulator (icubSim) is running
        // 2 - the gaze server iKinGazeCtrl is running and
        //     launched with the following options:
        //     --from configSim.ini
        Property optGaze;
        optGaze.put("device","gazecontrollerclient");
        optGaze.put("remote","/iKinGazeCtrl");
        optGaze.put("local","/tracker/gaze");

        if (!clientGaze.open(optGaze))
        {
            yError()<<"Unable to open the Gaze Controller";
            return false;
        }

        // open the view
        clientGaze.view(igaze);

        // latch the controller context in order to preserve
        // it after closing the module
        // the context contains the tracking mode, the neck limits and so on.
        igaze->storeContext(&startup_context_id);

        // set trajectory time
        igaze->setNeckTrajTime(0.6);
        igaze->setEyesTrajTime(0.4);

        port.open("/tracker/target:i");

        return true;
    }

    virtual void run()
    {
        Bottle *pTarget=port.read(false);
        if (pTarget!=NULL)
        {
            if (pTarget->size()>2)
            {
                if (pTarget->get(2).asInt()!=0)
                {
                    Vector px(2);
                    px[0]=pTarget->get(0).asDouble();
                    px[1]=pTarget->get(1).asDouble();

                    // track the moving target within the camera image
                    igaze->lookAtMonoPixel(0,px);   // 0: left image plane is used, 1: for right
                    yInfo()<<"gazing at pixel: "<<px.toString(3,3);
                }
            }
        }
    }

    virtual void threadRelease()
    {
        // we require an immediate stop
        // before closing the client for safety reason
        igaze->stopControl();

        // it's a good rule to restore the controller
        // context as it was before opening the module
        igaze->restoreContext(startup_context_id);

        clientGaze.close();
        port.close();
    }
};


class CtrlModule: public RFModule
{
protected:
    CtrlThread thr;

public:
    virtual bool configure(ResourceFinder &rf)
    {
        // retrieve command line options
        double period=rf.check("period",Value(0.02)).asDouble();

        // set the thread rate that is an integer accounting for [ms]
        thr.setRate(int(period*1000.0));

        return thr.start();
    }

    virtual bool close()
    {
        thr.stop();
        return true;
    }

    virtual double getPeriod()
    {
        return 1.0;
    }

    virtual bool updateModule()
    {
        return true;
    }
};



int main(int argc, char *argv[])
{
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError()<<"YARP doesn't seem to be available";
        return 1;
    }

    ResourceFinder rf;
    rf.configure(argc,argv);

    CtrlModule mod;
    return mod.runModule(rf);
}
