#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"

//#include "ModelIo.h"
#include "ModelSourceAssimp.h" //FIXME: including ModelIo.h only breaks the build
#include "SkinnedMesh.h"
#include "SkinnedVboMesh.h"

class ArmyDemoApp : public AppNative {
public:
	void setup();
	
	void resize();
	void keyDown( KeyEvent event );
	void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void fileDrop( FileDropEvent event );
	
	void update();
	void draw();
private:
	SkinnedVboMeshRef				mSkinnedVboMesh;
	
	MayaCamUI						mMayaCam;
	float							mMouseHorizontalPos;
	float							rotationRadius;
	Vec3f							mLightPos;
	
	int								mMeshIndex;
	float							mTime, mFps;
	params::InterfaceGl				mParams;
	bool mDrawSkeleton, mDrawMesh, mDrawRelative, mEnableSkinning, mEnableWireframe;
};

void ArmyDemoApp::setup()
{
	rotationRadius = 20.0f;
	mLightPos = Vec3f(0, 20.0f, 0);
	mMouseHorizontalPos = 0;
	mMeshIndex = 0;
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 250 ) );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mDrawMesh = true;
	mParams.addParam( "Draw Mesh", &mDrawMesh );
	mDrawSkeleton = false;
	mParams.addParam( "Draw Skeleton", &mDrawSkeleton );
	mDrawRelative = false;
	mParams.addParam( "Relative/Abolute skeleton", &mDrawRelative );
	mEnableSkinning = true;
	mParams.addParam( "Skinning", &mEnableSkinning );
	mEnableWireframe = false;
	mParams.addParam( "Wireframe", &mEnableWireframe );
	
	//	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
	
	mSkinnedVboMesh = make_shared<SkinnedVboMesh>( loadModel( getAssetPath( "maggot3.md5mesh" ) ) );
}

void ArmyDemoApp::fileDrop( FileDropEvent event )
{
	try {
		fs::path modelFile = event.getFile( 0 );
		mSkinnedVboMesh = make_shared<SkinnedVboMesh>( loadModel( modelFile ) );
	}
	catch( ... ) {
		console() << "unable to load the asset!" << std::endl;
	};
}

void ArmyDemoApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_m ) {
		mDrawRelative = !mDrawRelative;
	} else if( event.getCode() == KeyEvent::KEY_s ) {
		mEnableSkinning = !mEnableSkinning;
	} else if( event.getCode() == KeyEvent::KEY_UP ) {
		mMeshIndex++;
	} else if( event.getCode() == KeyEvent::KEY_DOWN ) {
		mMeshIndex = math<int>::max(mMeshIndex - 1, 0);
	}
}

void ArmyDemoApp::mouseMove( MouseEvent event )
{
	mMouseHorizontalPos = float( event.getX() );
}

void ArmyDemoApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void ArmyDemoApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void ArmyDemoApp::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

void ArmyDemoApp::update()
{
//	mSkinnedVboMesh->update( time, mEnableSkinning );
	mFps = getAverageFps();
	mTime = mSkinnedVboMesh->getSkeleton()->mAnimationDuration * mMouseHorizontalPos / getWindowWidth();
	
	mLightPos.x = rotationRadius * math<float>::sin( app::getElapsedSeconds() );
	mLightPos.z = rotationRadius * math<float>::cos( app::getElapsedSeconds() );
}

void ArmyDemoApp::draw()
{
	// clear out the window with black
	gl::clear( Color::black() );
	gl::setMatrices( mMayaCam.getCamera() );
	gl::translate(0, -5.0f, 0.0f);
	
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	light.setAmbient( Color::white() );
	light.setDiffuse( Color::white() );
	light.setSpecular( Color::white() );
	light.lookAt( mLightPos, Vec3f::zero() );
	light.update( mMayaCam.getCamera() );
	light.enable();
//
//	
//	gl::drawVector(mLightPos, Vec3f::zero() );
//	
	gl::enable( GL_LIGHTING );
	gl::enable( GL_NORMALIZE );
	
	gl::scale(0.1f, 0.1f, 0.1f);

	
	if ( mEnableWireframe )
		gl::enableWireframe();
	if( mDrawMesh ) {
		for(int i=-10; i<=10; i+=2) {
			for(int j=-10; j<=10; j+=2 ) {
				gl::pushModelView();
				gl::translate(40.0f*i, 0, 40.0f*j);
				mSkinnedVboMesh->update( mTime + 2.0f*( i*20.0f + j*20.0f )/400.0f, mEnableSkinning );
				mSkinnedVboMesh->draw();
				gl::popModelView();
			}
		}
//		mSkinnedVboMesh->draw();
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	if( mDrawSkeleton) {
		mSkinnedVboMesh->getSkeleton()->draw(mDrawRelative);
	}
	
	params::InterfaceGl::draw();
	
}

CINDER_APP_NATIVE( ArmyDemoApp, RendererGl )