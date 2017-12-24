#include "ofMain.h"
#include "ofApp.h"
//#include "ofGLProgrammableRenderer.h"

//========================================================================
int main( ){


	//ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context


	//ofGLWindowSettings s;
	//s.setGLVersion(3, 2);
	//s.width = 1024;//画面サイズ
	//s.height = 768;//画面サイズ
	//ofCreateWindow(s);

	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1280, 960, OF_WINDOW);			// <-------- setup the GL context
	//ofSetupOpenGL(&window, 1024, 768, OF_FULLSCREEN);			// <-------- setup the GL context
	

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
