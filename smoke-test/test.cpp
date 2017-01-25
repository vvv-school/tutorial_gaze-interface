/*
 * Copyright (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Ugo Pattacini <ugo.pattacini@iit.it>
 * CopyPolicy: Released under the terms of the GNU GPL v3.0.
*/

#include <cmath>
#include <algorithm>

#include <rtf/yarp/YarpTestCase.h>
#include <rtf/dll/Plugin.h>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>

using namespace std;
using namespace RTF;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;

/**********************************************************************/
class TestTutorialGazeInterface : public YarpTestCase
{
    BufferedPort<Bottle> port;

public:
    /******************************************************************/
    TestTutorialGazeInterface() :
        YarpTestCase("TestTutorialGazeInterface")
    {
    }

    /******************************************************************/
    virtual ~TestTutorialGazeInterface()
    {
    }

    /******************************************************************/
    virtual bool setup(yarp::os::Property& property)
    {
        port.open("/"+getName()+"/target:i");
        RTF_ASSERT_ERROR_IF(Network::connect("/detector/target",port.getName()),
                            "Unable to connect to target!");

        return true;
    }

    /******************************************************************/
    virtual void tearDown()
    {
        port.close();
    }

    /******************************************************************/
    virtual void run()
    {
        Vector c(2);
        c[0]=320.0/2.0;
        c[1]=240.0/2.0;

        double mean_x=0;
        double stdev_x=0;
        int N=0;
        
        Time::delay(5.0);

        RTF_TEST_REPORT("Checking target position in the image plane");
        for (double t0=Time::now(); Time::now()-t0<10.0;)
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

                        double d=norm(c-px);
                        mean_x+=d;
                        stdev_x+=d*d;

                        N++;
                    }
                }
            }

            Time::delay(0.1);
        }

        mean_x/=N;
        stdev_x=sqrt(stdev_x/N-mean_x*mean_x);

        RTF_TEST_REPORT(Asserter::format("mean distance from the image center = %g [m]",mean_x));
        RTF_TEST_REPORT(Asserter::format("stdev distance from the image center = %g [m]",stdev_x));
        RTF_TEST_CHECK((mean_x<30.0) && (stdev_x<10.0),"Tracking Test Passed!");
    }
};

PREPARE_PLUGIN(TestTutorialGazeInterface)
