
#include "testApp.h"
#include "stdio.h"

int buttonCount = 22;
float smoothAmnt = 0.09;
int NUM_MODES = 6;

//--------------------------------------------------------------
testApp::testApp(){

}                                                  

//--------------------------------------------------------------
void testApp::setup(){

	
	//---- setup standard application settings
	
	ofSetFullscreen(true);
	
	//	ofSetVerticalSync(true);  
	
	mode = MODE_TRACKING;
	TM.setup();
	CM.setup();
	typeScene.setup();
	eyeSmoothed.set(0,0,0);
	
	eyeApp.setup(0, 0, 1024, 768);
	
	ponger.setup();

	
	
	BT.setup("catch me!", 50,50,180,180);
	BT.setRetrigger(true);
	
	timeSince = 0;
	bMouseSimulation = false;
	bMouseEyeInputSimulation = false;
	
}


//--------------------------------------------------------------
void testApp::update(){

	ofBackground(30,30,30);
	
	// update the tracking manager (and internally, its input manager)
	TM.update();
	
	
	// update the calibration manager
	CM.update();
	
	// record some points if we are in auto mode
	if (CM.bAutomatic == true && CM.bAmInAutodrive == true && CM.bInAutoRecording){
		
		if (TM.bGotAnEyeThisFrame()){	
			ofPoint trackedEye = TM.getEyePoint();
			CM.fitter.registerCalibrationInput(trackedEye.x,trackedEye.y);
			CM.inputEnergy = 1;
		}
	}
	
	if (!bMouseSimulation){
	// smooth eye data in...
		if (CM.fitter.bBeenFit){
			
			ofPoint trackedEye;
			
			if (bMouseEyeInputSimulation) {
				trackedEye.x = mouseX;
				trackedEye.y = mouseY;
			} else {
				trackedEye = TM.getEyePoint();
			}

			screenPoint = CM.fitter.getCalibratedPoint(trackedEye.x, trackedEye.y);
			eyeSmoothed.x = CM.smoothing * eyeSmoothed.x + (1-CM.smoothing) * screenPoint.x;
			eyeSmoothed.y = CM.smoothing * eyeSmoothed.y + (1-CM.smoothing) * screenPoint.y;
		}
		
	} else {
		
		eyeSmoothed.x = mouseX;
		eyeSmoothed.y = mouseY;
	}

	
	if (mode == MODE_TEST){
		ofHideCursor();
		ofPoint pt = eyeSmoothed;
		if (BT.update(pt.x, pt.y)){
			BT.x = ofRandom(100,ofGetWidth()-100);
			BT.y = ofRandom(100,ofGetHeight()-100);
		}
	}
	
	if( mode == MODE_DRAW ){
		ofHideCursor();

		ofPoint pt = eyeSmoothed;
		//if( ofGetElapsedTimef() - timeSince >= 1.0/8.0 ){

			eyeApp.update();
			
			//To test with mouse comment this line and uncomment the line below
			//if( TM.bGotAnEyeThisFrame() || bMouseSimulation ){
				eyeApp.updatePoint( pt.x, pt.y, 1.0);
			//}
			//eyeApp.update( mouseX, mouseY );
			
		//	timeSince = ofGetElapsedTimef();
		//}
	}
	
	if (mode == MODE_TYPING){
		if(typeScene.panel.hidden) ofHideCursor();
		else ofShowCursor();
		
		ofPoint pt = eyeSmoothed;
		typeScene.update(pt.x, pt.y);
	}
	
	if (mode == MODE_PONG){
		ofHideCursor();

		ofPoint pt = eyeSmoothed;
		ponger.update(pt.x, pt.y);
	}
	
	if(mode == MODE_TRACKING){
		ofShowCursor();
	}
	
}

//--------------------------------------------------------------
void testApp::draw(){

	
	ofSetColor(255, 255, 255);
	
	if (mode == MODE_TRACKING)			TM.draw();
	if (mode == MODE_CALIBRATING)		CM.draw();
	if (mode == MODE_TEST)				BT.draw();
	if (mode == MODE_DRAW )				eyeApp.draw();
	if (mode == MODE_TYPING)			typeScene.draw();
	if (mode == MODE_PONG)				ponger.draw();
		
	// draw a green dot to see how good the tracking is:
	if (CM.fitter.bBeenFit || bMouseSimulation){
		if( mode != MODE_DRAW ){	
			ofSetColor(0,255,0,120);
			ofFill();
			ofCircle(eyeSmoothed.x, eyeSmoothed.y, 20);
		}
	}
	
	if (TM.IM.bRecord) {
		ofSetColor(255, 0, 0);
		ofFill();
		ofRect(0, ofGetHeight()-10, 10, 10);
	}
	
	if( mode == MODE_DRAW ){
		ofEnableAlphaBlending();
		ofSetColor(255, 255, 255, 120);
		TM.drawInput(0, ofGetHeight()-TM.IM.height/4, TM.IM.width/4, TM.IM.height/4, TM.IM.width/4, ofGetHeight()-TM.IM.height/4, TM.IM.width/4, TM.IM.height/4);	
		ofDisableAlphaBlending();
	}
	else if (mode != MODE_TRACKING && mode != MODE_DRAW ) {
		ofEnableAlphaBlending();
		ofSetColor(255, 255, 255, 120);
		TM.drawInput(0, 0, TM.IM.width/4, TM.IM.height/4, TM.IM.width/4, 0, TM.IM.width/4, TM.IM.height/4);	
		ofDisableAlphaBlending();
	}
	
	//ofSetColor(255, 255, 255);
	//ofDrawBitmapString("FrameRate: " + ofToString(ofGetFrameRate(), 5), 1, ofGetHeight() - 20);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
	//Knob Key Mapping
	//'+' Rotate Right
	//'-' Rotate Left
	//'@' Push
	//'#' Long Push
	
	if( key == ' ' && mode == MODE_TYPING ){
		typeScene.sendToRobot();
	}
	
	if( key == ' ' && mode == MODE_TRACKING ){
		float thresh = TM.panel.getValueF("THRESHOLD_EYEPOS");
		thresh += 15;
		if( thresh >= 110 ){
			thresh = 24;
		}
		TM.panel.setValueF("THRESHOLD_EYEPOS", thresh);
	}
	
	if( key == '\\' && mode == MODE_DRAW){
		eyeApp.drawScene.checkOffset();
	}
	
	if( key == '+' || key == '=' ){
		if( mode == MODE_DRAW){
			smoothAmnt += 0.005;
			if( smoothAmnt > 0.999 ){
				smoothAmnt = 0.999;
			}
		}
	}else if( key == '-'){
		if( mode == MODE_DRAW){
			smoothAmnt -= 0.005;
			if( smoothAmnt < 0.001 ){
				smoothAmnt = 0.001;
			}
		}
	}
	
	switch (key){
			
		case	OF_KEY_LEFT:
			mode --;
			if( mode < 0 ){
				mode = NUM_MODES-1;
			}
			if( mode == MODE_DRAW ){
				eyeApp.drawScene.clearOffset();
			}			
		break;
		
		case	OF_KEY_RIGHT:
		case	OF_KEY_RETURN:
		case	'@':
			mode ++;
			mode %= NUM_MODES; // number of modes;
			
			if( mode == MODE_DRAW ){
				eyeApp.drawScene.clearOffset();
			}
			break;
			
		case	'm':
		case	'M':
			bMouseSimulation = !bMouseSimulation;
			break;
			
		case	'r':
		case	'R':
			if (TM.IM.bRecord) TM.IM.stopRecord();
			else TM.IM.startRecord();
			break;
			
		case	'n':
		case	'N':
			bMouseEyeInputSimulation = !bMouseEyeInputSimulation;
			break;
			

		case	'f':
		case	'F':
			ofToggleFullscreen();
			break;
			
		case	'c':
		case	'C':
			TM.setOriginalPosition();
			break;
			
		case 'x':
		case 'X':
			TM.bOriginalPositon = false;
			break;
			
			
	}

	if (mode == MODE_TYPING){
		if( key == 'D' ){
			typeScene.panel.toggleView();
		}
	}
	
	if( mode == MODE_DRAW ){
		eyeApp.keyPressed(key);
	}	
	
	if (mode == MODE_CALIBRATING){
		CM.keyPressed(key);
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
	if (mode == MODE_TRACKING)			TM.mouseDragged(x, y, button);
	if (mode == MODE_CALIBRATING)		CM.mouseDragged(x, y, button);
	if (mode == MODE_DRAW)				eyeApp.mouseDragged(x, y, button);	
	if (mode == MODE_TYPING )			typeScene.panel.mouseDragged(x, y, button);

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

	
	if (mode == MODE_TRACKING)			TM.mousePressed(x, y, button);
	if (mode == MODE_CALIBRATING)		CM.mousePressed(x, y, button);
	if (mode == MODE_DRAW)				eyeApp.mousePressed(x, y, button);
	if (mode == MODE_TYPING )			typeScene.panel.mousePressed(x, y, button);

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
	if (mode == MODE_TRACKING)			TM.mouseReleased();
	if (mode == MODE_CALIBRATING)		CM.mouseReleased(x,y,button);
	if (mode == MODE_DRAW)				eyeApp.mouseReleased(x, y, button);
	if (mode == MODE_TYPING )			typeScene.panel.mouseReleased();
}

//--------------------------------------------------------------
void testApp::resized(int w, int h){

}
