#include "ofApp.h"

#include "Header.h"


void ofApp::image_buff_clear() {

	unsigned char *data = image_buff.getPixels();
	int ww = image_buff.getWidth();
	for (int y = 0; y < image_buff.getHeight(); y++) {
		for (int x = 0; x < image_buff.getWidth(); x++) {
			int val = 0;
			data[y *ww  + x] = val;
		}
	}
	image_buff.update();

}


//--------------------------------------------------------------
void ofApp::setup() {
	//ファイルの数
	long nn_max = 0;
	string fnamelist[1000];
	string dir, ff, ext;
	dir = ".\\data\\images\\";
	ext = "png";
	get_filelist(dir, fnamelist, nn_max, ext);

	page_status = 0;

	ofBackground(0, 0, 0);
	currentFrame = 0;
	max_frame = nn_max/2;
	//連番がふられた画像を順番に読み込み
	for (int i = 0; i < max_frame; i++) {
		//ファイル名を一時的に格納する文字列
		char char1[1024];
		sprintf(char1, "images/image%03d_A.png", i + 1);
		myImage_A[i].loadImage(char1);
		cout << char1 <<":read"<< endl;
		char char2[1024];
		sprintf(char2, "images/image%03d_B.png", i + 1);
		myImage_B[i].loadImage(char2);
		cout << char2 << ":read" << endl;

	}

	//srcImg.load("image1.jpg");
	//dstImg.load("image2.jpg");
	brushImg.load("brush.png");

	int width = myImage_A[0].getWidth();
	int height = myImage_B[0].getHeight();


	image_buff.allocate(width, height, OF_IMAGE_GRAYSCALE);
	image_buff_clear();

	depth_width = 640;
	depth_height = 480;
	depth_Inv.allocate(depth_width, depth_height, OF_IMAGE_GRAYSCALE);

	gui.setup();
	gui.add(doFullScreen.set("fullscreen (F)", false));
	gui.add(Skeleton_draw.set("Skeleton_draw", false));
	gui.add(depth_draw.set("depth_draw", false));
	gui.add(buff_draw.set("buff_draw", false));
	gui.add(p1.setup("Power", 50, 0, 200));
	gui.add(p2.setup("pixel_ratio", 0.7, 0, 1.0));
	gui.add(p3.setup("AnserPage_Time", 10.0, 0, 30.0));
	gui.add(p4.setup("QuationPage_Time", 50.0, 0, 120.0));

	
	doFullScreen.addListener(this, &ofApp::setFullScreen);

	//gui.add(p8.setup("py", 0, -50, +50));

	gui.loadFromFile("settings.xml");

	kinect.initSensor(0);
	//kinect.initIRStream(640, 480);
	kinect.initColorStream(width, height, true);
	kinect.initDepthStream(depth_width, depth_height, true);
	kinect.initSkeletonStream(true);
	//simple start
	kinect.start();

	//ofDisableAlphaBlending(); //Kinect alpha channel is default 0;
	ofEnableAlphaBlending();

	ofSetCircleResolution(64);
		
	hasSkeleton = true;

	maskFbo.allocate(1920, 1680);
	fbo.allocate(1920, 1680);





#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/alphamask.vert", "shaders_gles/alphamask.frag");
#else
	if (ofIsGLProgrammableRenderer()) {
		string vertex = "#version 150\n\
    	\n\
		uniform mat4 projectionMatrix;\n\
		uniform mat4 modelViewMatrix;\n\
    	uniform mat4 modelViewProjectionMatrix;\n\
    	\n\
    	\n\
    	in vec4  position;\n\
    	in vec2  texcoord;\n\
    	\n\
    	out vec2 texCoordVarying;\n\
    	\n\
    	void main()\n\
    	{\n\
	        texCoordVarying = texcoord;\
    		gl_Position = modelViewProjectionMatrix * position;\n\
    	}";
		string fragment = "#version 150\n\
		\n\
		uniform sampler2DRect tex0;\
		uniform sampler2DRect maskTex;\
        in vec2 texCoordVarying;\n\
		\
        out vec4 fragColor;\n\
		void main (void){\
		vec2 pos = texCoordVarying;\
		\
		vec3 src = texture(tex0, pos).rgb;\
		float mask = texture(maskTex, pos).r;\
		\
		fragColor = vec4( src , mask);\
		}";
		shader.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
		shader.bindDefaults();
		shader.linkProgram();
	}
	else {
		string shaderProgram = "#version 120\n \
		#extension GL_ARB_texture_rectangle : enable\n \
		\
		uniform sampler2DRect tex0;\
		uniform sampler2DRect maskTex;\
		\
		void main (void){\
		vec2 pos = gl_TexCoord[0].st;\
		\
		vec3 src = texture2DRect(tex0, pos).rgb;\
		float mask = texture2DRect(maskTex, pos).r;\
		\
		gl_FragColor = vec4( src , mask);\
		}";
		shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);
		shader.linkProgram();
	}
#endif

	// otherwise it will bring some junk with it from the memory    
	maskFbo.begin();
	ofClear(0, 0, 0, 255);
	maskFbo.end();

	fbo.begin();
	ofClear(0, 0, 0, 255);
	fbo.end();

	bBrushDown = true;

	A_millis = ofGetElapsedTimeMillis();
	Q_millis = ofGetElapsedTimeMillis();

	verdana14.load("verdana.ttf", 14, true, true);
	verdana14.setLineHeight(30.0f);
	verdana14.setLetterSpacing(1.037);
}

//--------------------------------------------------------------
void ofApp::update() {
	kinect.update();
	depthImage = kinect.getDepthPixelsRef();

	// Adding temporal Force

	if (kinect.isNewSkeleton()) {
		for (int i = 0; i < kinect.getSkeletons().size(); i++) {
			if (kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HEAD) != kinect.getSkeletons().at(i).end()) {
				// just get the first one
				SkeletonBone headBone = kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HEAD)->second;
				SkeletonBone lHandBone = kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HAND_LEFT)->second;
				SkeletonBone rHandBone = kinect.getSkeletons().at(i).find(NUI_SKELETON_POSITION_HAND_RIGHT)->second;
				ofVec3f hb(headBone.getScreenPosition().x, headBone.getScreenPosition().y, 0);
				head = head.getInterpolated(hb, 0.5);
				head.z = ofInterpolateCosine(head.z, headBone.getStartPosition().x, 0.5) + 0.1;

				ofVec3f lhb(lHandBone.getScreenPosition().x, lHandBone.getScreenPosition().y, 0);
				lHand = lHand.getInterpolated(lhb, 0.5);
				lHand.z = ofInterpolateCosine(lHand.z, lHandBone.getStartPosition().x, 0.5);

				ofVec3f rhb(rHandBone.getScreenPosition().x, rHandBone.getScreenPosition().y, 0);
				rHand = rHand.getInterpolated(rhb, 0.5);
				rHand.z = ofInterpolateCosine(rHand.z, rHandBone.getStartPosition().x, 0.5);
				//hasSkeleton = true;
			}
		}
	}


	// MASK (frame buffer object)
	maskFbo.begin();
	if (bBrushDown) {
		if ((rHand.x != 0) && (rHand.y != 0))
			brushImg.draw(rHand.x - 50, rHand.y - 50, p1, p1);
	}
	maskFbo.end();

	// HERE the shader-masking happends
	//
	fbo.begin();
	// Cleaning everthing with alpha mask on 0 in order to make it transparent for default
	ofClear(0, 0, 0, 0);
	shader.begin();
	shader.setUniformTexture("maskTex", maskFbo.getTexture(), 1);
	float imgW = myImage_B[currentFrame].getWidth(), imgH = myImage_B[currentFrame].getHeight();
	myImage_B[currentFrame].draw(0, 0, ratioMin*imgW, ratioMin*imgH);

	shader.end();
	fbo.end();

	static int currentFrame_old = currentFrame;

	if (currentFrame_old == currentFrame) {
		unsigned char *data_buff = image_buff.getPixels();
		int circle_r = p1 / ratioMin;
		for (int y = 0; y < image_buff.getHeight(); y++) {
			for (int x = 0; x < image_buff.getWidth(); x++) {
				int imgW = image_buff.getWidth();
				int circle_x = rHand.x - x;
				int circle_y = rHand.y - y;
				if ((circle_x*circle_x + circle_y*circle_y < circle_r*circle_r) && (rHand.x != 0) && (rHand.y != 0)) {
					int val = 255;
					data_buff[y * imgW + x] = val;
				}
			}
		}
		image_buff.update();
		currentFrame_old = currentFrame;
	}
	else {
		image_buff_clear();
		currentFrame_old = currentFrame;
		pixel_ratio = 0;
		Q_millis = ofGetElapsedTimeMillis();
	}

	int pixel_cnt = 0;
	unsigned char *data_buff = image_buff.getPixels();
	for (int y = 0; y < image_buff.getHeight(); y++) {
		for (int x = 0; x < image_buff.getWidth(); x++) {
			int imgW = image_buff.getWidth();
			if (data_buff[y * imgW + x] == 255) pixel_cnt++;

		}
	}
	pixel_ratio = (double)pixel_cnt / (image_buff.getWidth()* image_buff.getHeight());

//	cout << pixel_ratio << endl;
	char buf[100];
	sprintf(buf, "%f", pixel_ratio);
	ofDrawBitmapString(buf, 20, 20);

	//ある程度消せている
	if (pixel_ratio > p2) {
		page_status = 1;
		A_millis = ofGetElapsedTimeMillis();
		image_buff_clear();
	}


	if (page_status == 1) {	//答えのページへ遷移
		int n_millis = ofGetElapsedTimeMillis();
		time_msec_A = n_millis - A_millis;

		if (n_millis - A_millis < p3 * 1000) {//ある程度回答を見せる。
			maskFbo.begin();
			ofClear(255, 255, 255, 255);
			maskFbo.end();
		}

		else {									//次の問題へ行く。
			currentFrame++;
			if (currentFrame >= max_frame)currentFrame = 0;

			maskFbo.begin();
			ofClear(0, 0, 0, 255);
			maskFbo.end();

			page_status = 0;
		}
	}


	//問題をずっと出している
	int n_millis = ofGetElapsedTimeMillis();
	time_msec_Q = n_millis - Q_millis;
	//	cout << n_millis - Q_millis << endl;
	if (n_millis - Q_millis > p4 * 1000) {
		Q_millis = ofGetElapsedTimeMillis();

		maskFbo.begin();
		ofClear(0, 0, 0, 255);
		maskFbo.end();
		
		image_buff_clear();
		
		currentFrame++;
		if (currentFrame >= max_frame)currentFrame = 0;
	}


}




void ofApp::draw() {


	ofSetColor(255, 255);

	float winW = ofGetWidth(), winH = ofGetHeight();
	float imgW = myImage_A[currentFrame].getWidth(), imgH = myImage_A[currentFrame].getHeight();

	float ratioW = winW / imgW,
		ratioH = winH / imgH;

	if (ratioW < ratioH)
		ratioMin = ratioW;
	else
		ratioMin = ratioH;


	myImage_A[currentFrame].draw(0, 0, ratioMin*imgW, ratioMin*imgH);
	fbo.draw(0, 0);

	int width_D = 320;
	int height_D = 240;

	unsigned char *data = depthImage.getPixels();
	unsigned char *data2 = depth_Inv.getPixels();
	//  make it negative
	int bytes = depthImage.getWidth() * depthImage.getHeight();
	for (int k = 0; k < bytes; k++)
		data2[k] = 255 - data[k];
	//  must update after pixel manipulations
	depth_Inv.update();

	if (depth_draw == true)
		depth_Inv.draw(winW - width_D, winH - height_D, width_D, height_D);

	if (Skeleton_draw) {
		viewSkeletons();
	}
	if (buff_draw) {
		image_buff.draw(0, 0);
	}
	if (!bHide) {
		gui.draw();
	}


	if (page_status == 0) {
		ofSetColor(225);
		char moji[4096];
		double time = p4-time_msec_Q / 1000.0;
		sprintf(moji, "Next question  %.1lf sec", time);
		verdana14.drawString(moji, 20, 35);
	}
	if (page_status == 1) {
		ofSetColor(225);
		char moji[4096];
		double time = p3 - time_msec_A / 1000.0;
		sprintf(moji, "Next question %.1lf sec", time);
		verdana14.drawString(moji, 20, 35);
	}

	char moji[4096];
	sprintf(moji, "Eraser %.1lf %%", pixel_ratio*100.0);
	verdana14.drawString(moji, 20, 60);

	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	maskFbo.begin();
	ofClear(0, 0, 0, 255);
	maskFbo.end();


	if (key == 'h') {
		bHide = !bHide;
	}
	else if (key == 's') {
		gui.saveToFile("settings.xml");
	}
	else if (key == 'l') {
		gui.loadFromFile("settings.xml");
	}


	else if (key == 'f') {
		doFullScreen.set(!doFullScreen.get());
	}

	else if (key == 'x') {
		currentFrame++;
		if (currentFrame >= max_frame)currentFrame = 0;
		page_status = 0;

	}
	else if (key == 'c') {
	
		maskFbo.begin();
		ofClear(255, 255, 255, 255);
		maskFbo.end();
		//page_status = 1;
	}


}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	static int flg = 5;
	if (flg == 0) {
		image_buff_clear();

		maskFbo.begin();
		ofClear(255, 255, 255, 255);

		maskFbo.end();
		flg = 1;
	}
	else if (flg == 1) {
		image_buff_clear();
		
		maskFbo.begin();
		ofClear(0, 0, 0, 255);

		maskFbo.end();
		currentFrame++;
		if (currentFrame >= max_frame)currentFrame = 0;
		flg = 0;
	}
	Sleep(50);

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
//--------------------------------------------------------------
void ofApp::viewSkeletons() {

	ofPushStyle();
	ofSetColor(255, 0, 0);
	ofSetLineWidth(3.0f);
	auto skeletons = kinect.getSkeletons();

	for (auto & skeleton : skeletons) {
		for (auto & bone : skeleton) {
			ofSetColor(0, 0, 255,100);
			//switch (bone.second.getTrackingState()) {
			//case SkeletonBone::Inferred:
			//	ofSetColor(0, 0, 255);
			//	break;
			//case SkeletonBone::Tracked:
			//	ofSetColor(0, 255, 0);
			//	break;
			//case SkeletonBone::NotTracked:
			//	ofSetColor(255, 0, 0);
			//	break;
			//}

			auto index = bone.second.getStartJoint();
			auto connectedTo = skeleton.find((_NUI_SKELETON_POSITION_INDEX)index);
			if (connectedTo != skeleton.end()) {
				//	if (index != 1) {
				ofLine(connectedTo->second.getScreenPosition()*ratioMin, bone.second.getScreenPosition()*ratioMin);
				//	}
			}

			ofCircle(bone.second.getScreenPosition()*ratioMin, 10.0f);
		}
	}
	ofPopStyle();
}
