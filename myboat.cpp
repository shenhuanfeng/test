// myboat.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "stdio.h"
#include <cmath>
#include <ctime>
#include <vuMath.h>
#include <vuAllocTracer.h>
#include <vuFile.h>
#include <vpApp.h>
#include <vpObject.h>
#include <vrFontFactory.h>
#include <vpFxBlade.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include"iostream"
#include "holLoct.h"
#include <GL/gl.h>	
#include "cirLocat.h"
#include "myKalman.h"
#include <vuImageFactory.h>

using namespace std;

vuAllocTracer m_allocTracer;
bool g_isFirstFrame=true;
bool lClickedIf = 0;
int test=0;
float xspeed = 6;
float yspeed = 8;
vuFile g_file_x,g_file_y,g_file_z,g_file_h,g_file_p,g_file_r;

// the main application class
class myApp : public vpApp,public vsChannel::Subscriber 
{
private:
	bool save;
public:

    /**
     * Constructor
     */
	myApp()
    {
        // open the data file for reading or writing depending on the mode
		
		/*m_airspeed = m_groundspeed = m_verticalspeed = 0;*/
		m_point[0] = -5699;
		m_point[1] = 4058;
		m_point[2] = 50;
		g_file_x.open("file_x.txt","wt");
		g_file_y.open("file_y.txt","wt");
		g_file_z.open("file_z.txt","wt");
		g_file_h.open("file_h.txt","wt");
		g_file_p.open("file_p.txt","wt");
		g_file_r.open("file_r.txt","wt");

		mykal.init_hnn();
		speedkal.init_hnn();
    }

    /**
     * Destructor
     */
    ~myApp() 
    {
        // close the file
       
        // unreference member variables which cache Vega Prime class instances
        //m_channel->unref();
		m_object->unref();
		m_blade->unref();
		m_DataChannel->unref();
		m_CameraChannel->unref();
		/*m_glCompPFD->unref();*/
		if(m_data != NULL)
			vuAllocArray<uchar >::free(m_data);
		
		g_file_x.close();
		g_file_y.close();
		g_file_z.close();
		g_file_h.close();
		g_file_p.close();
		g_file_r.close();
    }
    
    /**
     * Configure my app
     */
    int configure() 
	{

        // pre-configuration 

        // configure vega prime system first
        vpApp::configure();

        // post-configuration

        // Increase the reference count by one for all member variables which
        // cache the Vega Prime class instances.
        // 
        // All VSG/Vega Prime class instances are referenced counted. 
        // The initial reference count is 0 after the instance is created. 
        // When the reference count is smaller or equal to 0 in unref(), 
        // the instance will be deleted automatically. Increasing reference
        // count here for all member variables will guarantee that they will
        // not be deleted until myApp is deleted.
	
		m_object = vpObject::find("apache");
		assert(m_object);
		m_object->ref();
		m_boat = vpObject::find("waliangge");
		assert(m_boat);
		m_boat->ref();
		m_blade = vpFxBlade::find("blade");
		assert(m_blade);
		m_blade->ref();
		m_DataChannel = vpChannel::find("DataChannel");
		assert(m_DataChannel);
		m_DataChannel->ref();
		m_DataChannel->addSubscriber(vsChannel::EVENT_POST_DRAW,this);
		m_CameraChannel = vpChannel::find("CameraChannel");
		assert(m_CameraChannel);
		m_CameraChannel->ref();
		m_CameraChannel->addSubscriber(vsChannel::EVENT_POST_DRAW,this);
		//m_PanelChannel = vpChannel::find("PanelChannel");
		//assert(m_PanelChannel);
		//m_PanelChannel->ref();
		//m_PanelChannel->addSubscriber(vsChannel::EVENT_PRE_DRAW, this);
		//m_PanelChannel->addSubscriber(vsChannel::EVENT_POST_DRAW,this);
		//m_glCompPFD = vpGLStudioComponent::find("PFDGLStudioComponent");
		//assert(m_glCompPFD);
		//m_glCompPFD->ref();

		vrFontFactory *fontFactory = new vrFontFactory();
		m_font = (vrFont2D*)fontFactory->read( "system" ); 
		fontFactory->unref();

		//m_data = NULL; //vuAllocArray<uchar >::malloc(360*240*3);
		// m_imageFactory = new vuImageFactory();
		// save = true;
        return vsgu::SUCCESS;

    }

	virtual void notify(vsChannel::Event, const vsChannel *,
        vsTraversalCull *) {}

    /**
     * inherited pre / post draw notification method.  Note here that since
     * we're using this function for both channels that we'll need to add some
     * extra logic to organize our code here.
     */
    virtual void notify(vsChannel::Event event, const vsChannel *channel,
        vrDrawContext *context)
    {
		

        // for the binoculars channel we'll do all of our stencil stuff so
        // we can get the rounded outputs
        if (channel == m_DataChannel) 
		{

			vuVec3d point,rot,bpoint,brot;
			m_object->getPosition(&point[0],&point[1],&point[2]);
			m_object->getRotate(&rot[0],&rot[1],&rot[2]);
			m_boat->getPosition(&bpoint[0],&bpoint[1],&bpoint[2]);
			m_boat->getRotate(&brot[0],&brot[1],&brot[2]);

			float str[6]={point[0],point[1],point[2],rot[0], rot[1],  rot[2]};
			///sixvalue kalSix;
            mykal.timeRnew(str);

            if (event == vsChannel::EVENT_POST_DRAW)
			{
				srand(time(0));
				float xSpeed = xspeed+(sin(((float)rand() / RAND_MAX )*180))/10;  //为什么是这样？
				float ySpeed = yspeed+(sin(((float)rand() / RAND_MAX )*180))/10;
				float zSpeed = (sin(((float)rand() / RAND_MAX )*180))/10;
				float hSpeed = (sin(((float)rand() / RAND_MAX )*180))/10;
				float pSpeed = (sin(((float)rand() / RAND_MAX )*180))/10;
				float rSpeed = (sin(((float)rand() / RAND_MAX )*180))/10;
				m_text2.sprintf("     Position:   xPosition   yPosition   zPosition    Heading    Pitch     Roll");
				m_text3.sprintf("     True:      %12.1f%12.1f%12.1f%15.1f%11.1f%11.1f"
					,point[0],point[1],point[2],rot[0],rot[1],rot[2]);

				/*ture.x=point[0];
				ture.y=point[1];
				ture.z=point[2];
				ture.h=point[3];
				ture.p=point[4];
				ture.r=point[5];*/
                
				m_text4.sprintf("     Estimate:   %8.1f%12.1f%12.1f%15.1f%11.1f%11.1f",
					m_esti.x,m_esti.y,m_esti.z,m_esti.h,m_esti.p,m_esti.r);

				m_text5.sprintf("     Velocity:   xSpeed      ySpeed      zSpeed       hSpeed    pSpeed    rSpeed");
				m_text6.sprintf("     True:      %12.1f%12.1f%16.1f%18.1f%15.1f%14.1f"
					,-xSpeed,-ySpeed,zSpeed,hSpeed,pSpeed,rSpeed);

				/*ture.x=point[0];
				ture.y=point[1];
				ture.z=point[2];
				ture.h=point[3];
				ture.p=point[4];
				ture.r=point[5];*/
                
				m_text7.sprintf( "     Estimate:   %8.1f%12.1f%16.1f%18.1f%15.1f%14.1f",
					estSped.x,estSped.y,estSped.z,estSped.h,estSped.p,estSped.r);

				if ((s_vpKernel->getSimulationTime() > 108)  && (s_vpKernel->getSimulationTime()<210))
				{	
					char str[1024];
					sprintf(str,"%g   %g   %g\n",point[0],m_esti.x,s_vpKernel->getSimulationTime());
					g_file_x.writeString(str);
					sprintf(str,"%g   %g   %g\n",point[1],m_esti.y,s_vpKernel->getSimulationTime());
					g_file_y.writeString(str);
					sprintf(str,"%g   %g   %g\n",point[2],m_esti.z,s_vpKernel->getSimulationTime());
					g_file_z.writeString(str);
					sprintf(str,"%g   %g   %g\n",rot[0],m_esti.h,s_vpKernel->getSimulationTime());
					g_file_h.writeString(str);
					sprintf(str,"%g   %g   %g\n",rot[1],m_esti.p,s_vpKernel->getSimulationTime());
					g_file_p.writeString(str);
					sprintf(str,"%g   %g   %g\n",rot[2],m_esti.r,s_vpKernel->getSimulationTime());
					g_file_r.writeString(str);
				}
	
				context->pushElements(true);

				vuVec4<float> color(0.7f, 0.7f, 0.7f, 1.0f);
				//m_font->displayStringAt(context, m_text1.c_str(), color, -0.45f, 0.9f);
				m_font->displayStringAt(context, m_text2.c_str(), color, -0.9f, 0.8f);
				m_font->displayStringAt(context, m_text3.c_str(), color, -0.9f, 0.5f);
				m_font->displayStringAt(context, m_text5.c_str(), color, -0.9f, -0.2f);
				m_font->displayStringAt(context, m_text6.c_str(), color, -0.9f, -0.5f);

				if(s_vpKernel->getSimulationTime() > 100)
				{
					m_font->displayStringAt(context, m_text4.c_str(), color, -0.9f, 0.2f);
					m_font->displayStringAt(context, m_text7.c_str(), color, -0.9f, -0.8f);
				}
				//Sleep(30);
                context->popElements(false);
			
            }
		
        }
		if (channel == m_CameraChannel)
		{	
			double Dtime = s_vpKernel->getSimulationDeltaTime();
			//double Ctime = s_vpKernel->getSimulationTime();
			vuVec3d point,rot,bpoint,brot;
			m_object->getPosition(&point[0],&point[1],&point[2]);
			m_object->getRotate(&rot[0],&rot[1],&rot[2]);
			m_boat->getPosition(&bpoint[0],&bpoint[1],&bpoint[2]);
			m_boat->getRotate(&brot[0],&brot[1],&brot[2]);
			m_boat->setPosition(bpoint[0]-Dtime*xspeed,bpoint[1]-Dtime*yspeed,bpoint[2]);

			static int num = 1;
			static double sumtime = 0;
			/*char c_airSpeed[10], altitude[10], c_groundSpeed[10],c_verticalSpeed[10],
				c_heading[10],c_headingbug[10],c_roll[10],c_pitch[10];*/
			if(num == 5)
			{
				//m_airspeed = sqrt((point[0]-m_point[0])*(point[0]-m_point[0])+
				//	(point[1]-m_point[1])*(point[1]-m_point[1])+
				//	(point[2]-m_point[2])*(point[2]-m_point[2]))/sumtime;
				//m_groundspeed = sqrt((point[0]-m_point[0])*(point[0]-m_point[0])+
				//	(point[1]-m_point[1])*(point[1]-m_point[1]))/sumtime;
				//m_verticalspeed = sqrt((point[2]-m_point[2])*(point[2]-m_point[2]))/sumtime;

				//sprintf(c_airSpeed, "%8.2f", m_airspeed);
				//m_glCompPFD->setAttrib("AirSpeed", c_airSpeed);
				//sprintf(altitude, "%8.2f", point[2]);
				//m_glCompPFD->setAttrib("Altitude", altitude);
				//sprintf(c_groundSpeed, "%8.2f", m_groundspeed);
				//m_glCompPFD->setAttrib("GroundSpeed",c_groundSpeed);
				//sprintf(c_verticalSpeed, "%8.2f", m_verticalspeed);
				//m_glCompPFD->setAttrib("VerticalSpeed",c_verticalSpeed);
				num = 1;
				sumtime = 0;
				m_point = point;
			}

			num ++;
			sumtime = sumtime +Dtime;

			//sprintf(c_heading, "%8.2f", rot[0]);
			//m_glCompPFD->setAttrib("CurrentHeading", c_heading);
			//sprintf(c_headingbug, "%8.2f", rot[0]);
			//m_glCompPFD->setAttrib("HeadingBug", c_heading);
			//sprintf(c_pitch, "%8.2f", rot[1]);
			//m_glCompPFD->setAttrib("Pitch", c_pitch);
			//sprintf(c_roll, "%8.2f", rot[2]);
			//m_glCompPFD->setAttrib("Roll", c_roll);


			if(s_vpKernel->getSimulationTime() < 4)
			{
				//m_boat->setPosition(bpoint[0],bpoint[1]-Dtime*10,bpoint[2]);
			}
			else if (s_vpKernel->getSimulationTime() < 12)
			{
				//m_boat->setPosition(bpoint[0],bpoint[1]-Dtime*10,bpoint[2]);
				m_object->setPosition(point[0],point[1],point[2]+Dtime*60);
			}
				
			else if (s_vpKernel->getSimulationTime() < 20)
			{
				//m_boat->setPosition(bpoint[0],bpoint[1]-Dtime*10,bpoint[2]);
				m_object->setPosition(point[0]+Dtime*80,point[1]-Dtime*78.1126,point[2]+Dtime*60);
			}
			else if (s_vpKernel->getSimulationTime() < 38)
			{
				//m_boat->setPosition(bpoint[0],bpoint[1]-Dtime*10,bpoint[2]);
				m_object->setPosition(point[0]+Dtime*160,point[1]-Dtime*156.2252,point[2]);
			}
			else if (s_vpKernel->getSimulationTime() < 62.639625)
			{
				//m_boat->setPosition(bpoint[0],bpoint[1]-Dtime*10,bpoint[2]);
				m_object->setPosition(point[0]+Dtime*160,point[1]-Dtime*156.2252,point[2]-Dtime*17.071643);
			}
			else if (s_vpKernel->getSimulationTime() < 82.639625)
			{
				//m_boat->setPosition(bpoint[0],bpoint[1]-Dtime*10,bpoint[2]);
				m_object->setPosition(point[0]+Dtime*80,point[1]-Dtime*78.1126/*121.22822*/,point[2]-Dtime*17.071643);
			}
			else if ( s_vpKernel->getSimulationTime() < 100 )
			{
				static int t1=0;
				if(t1==0)
				{
					//m_boat->getPosition(&bpoint[0],&bpoint[1],&bpoint[2]);
					//cout<<bpoint[0]<<" "<<bpoint[1]<<" "<<bpoint[2]<<endl;
					//cout<<point[0]<<" "<<point[1]<<" "<<point[2]<<endl;
					point[0] = 3363;
					point[1] = -4790.4;
					point[2] = 247;
					t1++;

				}
				m_object->setPosition(point[0]-Dtime*(600/(100-82.639625)),point[1]-Dtime*(800/(100-82.639625)),point[2]-Dtime*7.891534);
				m_object->setRotate(rot[0]-Dtime*2.6315,rot[1],rot[2]);
			}
			else if(s_vpKernel->getSimulationTime() < 120)
			{
				m_object->setPosition(point[0]+Dtime*(0.0015-6),point[1]-Dtime*8,point[2]-Dtime);
				m_object->setRotate(-180,rot[1],rot[2]);
			}
			else if (s_vpKernel->getSimulationTime() < 140)
			{
				m_object->setPosition(point[0]-Dtime*(0.06+6),point[1]-Dtime*8,point[2]-Dtime);
				m_object->setRotate(rot[0],5,rot[2]);
				m_boat->setRotate(brot[0],5,rot[2]);
				//m_object->setRotate(rot[0]-Dtime*0.2,rot[1]-Dtime*0.2,rot[2]+Dtime*0.55);
			}
			else if (s_vpKernel->getSimulationTime() < 160)
			{
				m_object->setPosition(point[0]+Dtime*(0.06-6),point[1]-Dtime*7.97,point[2]-Dtime);
				m_object->setRotate(rot[0],5,rot[2]);
				m_boat->setRotate(brot[0],5,rot[2]);
				//m_object->setRotate(rot[0],rot[1],rot[2]);
				//m_object->setRotate(rot[0]+Dtime*0.4,rot[1]+Dtime*0.505,rot[2]-Dtime*1.0);
			}

			else if (s_vpKernel->getSimulationTime() < 180)
			{
				m_object->setPosition(point[0]-Dtime*(6-0.101),point[1]-Dtime*8.03+0.0003,point[2]-Dtime*0.9);
				//m_object->setRotate(rot[0],rot[1]+0.1,rot[2]);
				//m_boat->setRotate(brot[0],rot[1]+0.1,rot[2]);
				m_object->setRotate(rot[0],rot[1],rot[2]);
				//m_object->setRotate(rot[0]-Dtime*0.2,rot[1]-Dtime*0.3,rot[2]+Dtime*0.45);
			}
			else if(point[2]>16.3)
			{
				m_object->setPosition(point[0]-Dtime*xspeed,point[1]-Dtime*8,point[2]-Dtime);
				m_object->setRotate(-180,0,0);
			}
			else
			{
				m_object->setPosition(point[0]-Dtime*xspeed,point[1]-Dtime*8,point[2]);
				m_object->setRotate(-180,0,0);
			}
			
			if(s_vpKernel->getSimulationTime() < 100)
			{
				vuVec3d point,rot,bpoint,brot;
				m_object->getPosition(&point[0],&point[1],&point[2]);
				m_object->getRotate(&rot[0],&rot[1],&rot[2]);
				float kal[6]={point[0],point[1],point[2],rot[0],rot[1],rot[2]};
				estSped = speedkal.SpeedEsti(kal);				
			}
			
			if((s_vpKernel->getSimulationTime() > 100) && (s_vpKernel->getSimulationTime()<210) )
			{
				//static bool first = true;
				//if (first)
				//{			
				//	//m_panelwindow->addChannel(m_DataChannel);				
				//	first = false;
				//}
	
				if ( test % 5 == 0 )
				{
				    b = s_vpKernel->getSimulationTime();
					vuVec3d nePoint,neRot;
					m_boat->getPosition(&nePoint[0],&nePoint[1],&nePoint[2]);
					m_boat->getRotate(&neRot[0],&neRot[1],&neRot[2]);
					sixvalue nepoint;
					nepoint.x = nePoint[0];
					nepoint.y = nePoint[1];
					nepoint.z = nePoint[2];
					nepoint.h = neRot[0];
					nepoint.p = neRot[1];
					nepoint.r = neRot[2];
				// get the viewport
				int ox, oy, sx, sy;
				channel->getVrChannel()->getViewport(&ox, &oy, &sx, &sy);				
				// error checking
				// capture the frame

				m_data = vuAllocArray<uchar >::malloc(sx*sy*3);

 				glReadPixels(ox, oy, sx,sy , GL_RGB, GL_UNSIGNED_BYTE, m_data);
				// create an image 
				IplImage* image = cvCreateImageHeader(cvSize(sx,sy),IPL_DEPTH_8U,3);
 				cvSetData(image,m_data,sx*3);
				//cvCvtColor(image,image,CV_BGR2RGB);

				IplImage* image1 = cvCloneImage(image);
                //cvFlip(image, image1,0);
				int i,j,k;
				uchar *data,*data1;
				int height,width,step,channels;	
				height   = image->height;
				width    = image->width;
				step     = image->widthStep/sizeof(uchar);
				channels = image->nChannels;
				data     = (uchar *)image->imageData;	
				data1    = (uchar *)image1->imageData;
				//翻转图像
				for(i=0;i<height;i++)
					for(j=0;j<width;j++)
						for(k=0;k<channels;k++) 
							data[i*step+j*channels+k]=data1[(height-i)*step+j*channels+k];
				
				image->origin =1;  /*图像原点位置: 0表示顶-左结构,1表示底-左结构 */

				if(s_vpKernel->getSimulationTime()>180)
				{
					m_esti = esti.estiPosFunction(image,nepoint);
					float str[6]={m_esti.x,m_esti.y,m_esti.z,m_esti.h, m_esti.p,m_esti.r};					
					kalSix=mykal.measRnew(str);
					estSped = speedkal.SpeedEsti(str);
					estSped.x = estSped.x/(b-a);
					estSped.y = estSped.y/(b-a);
					estSped.z = estSped.z/(b-a);
					estSped.h = estSped.h/(b-a);
					estSped.p = estSped.p/(b-a);
					estSped.r = estSped.r/(b-a);

				}

				else
				{
                   m_esti = esti1.estiPosFunction(image,nepoint); 
				   float str[6]={m_esti.x,m_esti.y,m_esti.z,m_esti.h, m_esti.p,m_esti.r};
				   kalSix=mykal.measRnew(str);
				   float kal[6]={kalSix.x,kalSix.y,kalSix.z,kalSix.h,kalSix.p,kalSix.r};
				   estSped = speedkal.SpeedEsti(kal);
				   estSped.x = estSped.x/(b-a);
				   estSped.y = estSped.y/(b-a);
				   estSped.z = estSped.z/(b-a);
				   estSped.h = estSped.h/(b-a);
				   estSped.p = estSped.p/(b-a);
				   estSped.r = estSped.r/(b-a);
					
				}
				}
				test++;
				if(test > 0 &&((test - 1) % 5 == 0))
					 a = s_vpKernel->getSimulationTime();
		       }
			if (s_vpKernel->getSimulationTime()>210)
			{
				m_esti.x = point[0];
				m_esti.y = point[1];
				m_esti.z = point[2];
				m_esti.h = rot[0];
				m_esti.p = rot[1];
				m_esti.r = rot[2];

				kalSix.x = point[0];
				kalSix.y = point[1];
				kalSix.z = point[2];
				kalSix.h = rot[0];
				kalSix.p = rot[1];
				kalSix.r = rot[2];
				
				if(s_vpKernel->getSimulationTime()<205)
					m_blade->setRotationSpeed((205-s_vpKernel->getSimulationTime())/25.0);
			}
		}

 
//		if (event == vsChannel::EVENT_PRE_DRAW)
//		{
//                m_PanelChannel->getVrChannel()->setClearColor(0.25f, 0.4f, 0.2f, 0.0f);
////			m_PanelChannel->getVrChannel()->setClearColor(0.3f, 1.0f, 0.3f, 0.0f);
//                m_PanelChannel->getVrChannel()->apply(context);
//
//                glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
//        }
//
//        // in the post draw reset the color mask for normal rendering
//        else glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

}



	
public:
    sixvalue m_esti;
    sixvalue ture;
	sixvalue kalSix;
	sixvalue estSped;
	float a,b;
private:

    vpChannel *m_channel;
	vpObject  *m_object;
	vpObject  *m_boat;
	vpChannel *m_DataChannel,*m_CameraChannel;
	/*vpChannel *m_PanelChannel;*/
	vuField<vrFont2D *> m_font;
	//vuField<vrFont2D *> n_font;
	vuString m_text1;
	vuString m_text2;
	vuString m_text3;
	vuString m_text4;

	vuString m_text5;
	vuString m_text6;
	vuString m_text7;
	
	unsigned char *m_data;

    cirEstipos esti;
	estiPos esti1;
	myKalman mykal;
	myKalman speedkal;

	/*vpGLStudioComponent *m_glCompPFD;*/
	vuVec3d m_point;
	/*double m_airspeed,m_groundspeed,m_verticalspeed;*/
	vpFxBlade *m_blade;

	 vuField<uchar*,vuFieldTraitBinary>  mdata;
	 vuField<vuImageFactory*>    m_imageFactory;

	
};

int main(int argc, char *argv[])
{

    // initialize vega prime
    vp::initialize(argc, argv);

    // use the acf file to determine which mode to run in
  

    // create my app instance
    myApp *app = new myApp();
	//app->initializeModule("vppath");

    // load the acf file
    if (argc <= 1)
        app->define("h.acf");
    else app->define(argv[1]);

    // configure my app
    app->configure();

    // execute the main runtime loop
    app->run();

    // delete my app instance
    app->unref();

    // shutdown vega prime
    vp::shutdown();

    return 0;
}