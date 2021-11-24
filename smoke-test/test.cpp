/*
 * Copyright (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Ugo Pattacini <ugo.pattacini@iit.it>
 * CopyPolicy: Released under the terms of the GNU GPL v3.0.
*/

#include <cmath>
#include <algorithm>

#include <robottestingframework/dll/Plugin.h>
#include <robottestingframework/TestAssert.h>

#include <yarp/robottestingframework/TestCase.h>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>

using namespace std;
using namespace robottestingframework;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;

/**********************************************************************/
class TestTutorialGazeInterface : public yarp::robottestingframework::TestCase
{
    BufferedPort<Bottle> port;

public:
    /******************************************************************/
    TestTutorialGazeInterface() :
        yarp::robottestingframework::TestCase("TestTutorialGazeInterface")
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
        if (!Network::connect("/detector/target",port.getName()))
            ROBOTTESTINGFRAMEWORK_ASSERT_FAIL("Unable to connect to target!");

        Time::useNetworkClock("/clock");
        return true;
    }

    /******************************************************************/
    virtual void tearDown()
    {
        Time::useSystemClock();
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

        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Checking target position in the image plane");
        for (double t0=Time::now(); Time::now()-t0<10.0;)
        {
            Bottle *pTarget=port.read(false);
            if (pTarget!=NULL)
            {
                if (pTarget->size()>2)
                {
                    if (pTarget->get(2).asInt32()!=0)
                    {
                        Vector px(2);
                        px[0]=pTarget->get(0).asFloat64();
                        px[1]=pTarget->get(1).asFloat64();

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

        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("mean distance from the image center = %g [m]",mean_x));
        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("standard deviation = %g [m]",stdev_x));
        ROBOTTESTINGFRAMEWORK_TEST_CHECK((mean_x<25.0) && (stdev_x<25.0),"Tracking Test Passed!");
    }
};

ROBOTTESTINGFRAMEWORK_PREPARE_PLUGIN(TestTutorialGazeInterface)
