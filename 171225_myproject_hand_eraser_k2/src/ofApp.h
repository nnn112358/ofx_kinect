#pragma once
#include "ofxKinectCommonBridge.h"

#include "ofMain.h"
#include "ofxFluid.h"

#include "ofxGui.h"
//#include "ofxDatGui.h"

#define FRAMENUM 300 //ì«Ç›çûÇﬁâÊëúÇÃñáêî

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void viewSkeletons();

		ofxKinectCommonBridge kinect;

		ofImage kinectImage;
		ofImage depthImage;
		unsigned char * 	video_buff;
		ofTexture			videoTexture;

		unsigned char * 	depth_buff;
		//unsigned char * 	depth_Inv;
		ofTexture			depthTexture;
		ofImage depth_Inv;

		unsigned char * 	video_cut;
		ofTexture			videoTexture_cut;


		ofxFluid fluid;
		//ofxFluid fluid2;

		int     width, height;
		int     depth_width, depth_height;
		ofVec2f oldM;
		ofVec2f oldM2;

		ofxPanel gui;
		bool bHide;
		double dissipation_p1;
		
		ofParameter<bool> depth_draw;
		ofParameter<bool> Skeleton_draw;
		ofParameter<bool> buff_draw;


		ofxFloatSlider p1, p2, p3, p4, p5, p6,p7,p8;
		ofxFloatSlider rr2, gg2, bb2;
		ofxFloatSlider rr,gg,bb;
		ofVec3f head, lHand, rHand;
		bool hasSkeleton;
		
		ofParameter<bool>	doFullScreen;
		void				setFullScreen(bool& _value) { ofSetFullscreen(_value); }


		//ofImage     srcImg;
		//ofImage     dstImg;
		ofImage     brushImg;

		ofFbo       maskFbo;
		ofFbo       fbo;

		ofShader    shader;

		bool        bBrushDown;


		double	ratioMin;
		ofImage myImage_A[FRAMENUM];
		ofImage myImage_B[FRAMENUM];
		int currentFrame;
		int max_frame;

		ofImage image_buff;
		void image_buff_clear();
		double pixel_ratio;


		int page_status;//0:A,1:B
		int A_millis;
		int Q_millis;

		double time_msec_Q;
		double time_msec_A;
		string typeStr;

		ofTrueTypeFont	verdana14;


};
