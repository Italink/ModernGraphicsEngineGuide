#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Pass/QVideoRenderPass.h"
#include "Render/Pass/QPixelFilterRenderPass.h"
#include "Render/Pass/QBlurRenderPass.h"
#include "Render/Pass/QBloomRenderPass.h"
#include "Render/Pass/QToneMappingRenderPass.h"
#include "QSequence.h"
#include "Utils/QAudioProvider.h"
#include "Render/Pass/QGlslSandboxRenderPass.h"
#include "QEngineApplication.h"

class MySequence :public QSequence {
private:
	QRenderWidget mWidget;
	QSharedPointer<QAudioProvider> mAudioProvider;
	QSharedPointer<QSpectrumProvider> mSpectruomProvider;
    QSharedPointer<QFrameGraph> mStage0;
    QSharedPointer<QFrameGraph> mStage1;
public:
	MySequence()
        : mWidget(QRhiWindow::InitParams({QRhi::Vulkan}))
		, mAudioProvider(new QAudioProvider)
		, mSpectruomProvider(mAudioProvider->createSpectrumProvider())
	{
		mAudioProvider->setSource(QUrl::fromLocalFile(RESOURCE_DIR"/Audio/I Say Yeah.m4a"));

        mStage0 = QFrameGraph::Begin()
            .addPass(
            QGlslSandboxRenderPass::Create("GlslSandbox")
            .setShaderCode(R"(
                #ifdef GL_ES
                precision highp float;
                #endif

                const float numOct  = 5. ;  //number of fbm octaves
                float focus = 0.;
                float focus2 = 0.;
                #define pi  3.14159265

                float random(vec2 p) {
                    //a random modification of the one and only random() func
                    return fract( sin( dot( p, vec2(12., 90.)))* 5e5 );
                }

                mat2 rot2(float an){float cc=cos(an),ss=sin(an); return mat2(cc,-ss,ss,cc);}

                float noise(vec3 p) {
                    vec2 i = floor(p.yz);
                    vec2 f = fract(p.yz);
                    float a = random(i + vec2(0.,0.));
                    float b = random(i + vec2(1.,0.));
                    float c = random(i + vec2(0.,1.));
                    float d = random(i + vec2(1.,1.));
                    vec2 u = f*f*(3.-2.*f); 
    
                    return mix( mix(a,b,u.x), mix(c,d,u.x), u.y);
                }

                float fbm3d(vec3 p) {
                    float v = 0.;
                    float a = .5;
                    vec3 shift = vec3(focus - focus2);     //play with this
    
                    float angle = pi/1.3 + .03*focus;      //play with this

                    for (float i=0.; i<numOct; i++) {
                        v += a * noise(p);
                        p.xz = rot2(-angle)*p.xz ;
                        p = 2.*p + shift;
                        a *= .22*(1.+focus+focus2);  //this is the main modification that makes the fbm more interesting
                    }
                    return v;
                }

                void mainImage( out vec4 fragColor, in vec2 fragCoord )
                {

                    vec2 uv = (2.*fragCoord-iResolution.xy)/iResolution.y * 2.5;

                    float aspectRatio = iResolution.x / iResolution.y;

                    vec3 rd = normalize( vec3(uv, -1.2) );  
                    vec3 ro = vec3(0.,0.,0.); 
    
                    float delta = iTime / 1.3 ; 
        
                    rd.yz *= rot2(-delta );
                    rd.xz *= rot2(delta*3.);
                    vec3 p = ro + rd;

                    float bass = 1.8 + .8 * sin(iTime);  //used to be connected to audioContext.analyser
    
                    vec2 nudge = vec2( aspectRatio, 0.);

                    focus = length(uv + nudge);
                    focus = 1.8/(1.+focus) * bass;

                    focus2 = length(uv - nudge);
                    focus2 = 4.5/(1.+focus2*focus2) / bass;

                    vec3 q = vec3( fbm3d(p), fbm3d(p.yzx), fbm3d(p.zxy) ) ;

                    float f = fbm3d(p + q);
    
                    vec3 cc = q;
                    cc *= 20.*f;   

                    cc.r += 4.5*focus; cc.g+= 2.*focus; 
                    cc.b += 7.*focus2; cc.r-=3.5*focus2;    
                    cc /= 20.;
 

                    fragColor = vec4( cc,1.0);
                }
                // --------[ Original ShaderToy ends here ]---------- //

                void main(void)
                {
                    mainImage(gl_FragColor, gl_FragCoord.xy);
                }
			)")
            )
            .end("GlslSandbox", QGlslSandboxRenderPass::Out::Output);

        mStage1 = QFrameGraph::Begin()
            .addPass(
            QGlslSandboxRenderPass::Create("GlslSandbox")
            .setShaderCode(R"(
        #ifdef GL_ES
        precision highp float;
        #endif


        #define SAMPLE_COUNT 40
        #define PERIOD 1.

        // mouse toggle
        bool STRUCTURED;

        // cam moving in a straight line
        vec3 sundir;

        // LUT based 3d value noise
        float noise( in vec3 x )
        {
            vec3 p = floor(x);
            vec3 f = fract(x);
            f = f*f*(3.0-2.0*f);
    
            vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
            vec2 rg = vec2(0,0);
            return mix( rg.x, rg.y, f.z );
        }


        vec4 map( in vec3 p )
        {
	        float d = 0.1 + .8 * sin(0.6*p.z)*sin(0.5*p.x) - p.y;

            vec3 q = p;
            float f;
    
            f  = 0.5000*noise( q ); q = q*2.02;
            f += 0.2500*noise( q ); q = q*2.03;
            f += 0.1250*noise( q ); q = q*2.01;
            f += 0.0625*noise( q );
            d += 2.75 * f;

            d = clamp( d, 0.0, 1.0 );
    
            vec4 res = vec4( d );
    
            vec3 col = 1.15 * vec3(1.0,0.95,0.8);
            col += vec3(1.,0.,0.) * exp2(res.x*10.-10.);
            res.xyz = mix( col, vec3(0.7,0.7,0.7), res.x );
    
            return res;
        }


        // to share with unity hlsl
        #define float2 vec2
        #define float3 vec3
        #define fmod mod
        float mysign( float x ) { return x < 0. ? -1. : 1. ; }
        float2 mysign( float2 x ) { return float2( x.x < 0. ? -1. : 1., x.y < 0. ? -1. : 1. ) ; }

        // compute ray march start offset and ray march step delta and blend weight for the current ray
        void SetupSampling( out float2 t, out float2 dt, out float2 wt, in float3 ro, in float3 rd )
        {
            if( !STRUCTURED )
            {
                dt = float2(PERIOD,PERIOD);
                t = dt;
                wt = float2(0.5,0.5);
                return;
            }
    
            // the following code computes intersections between the current ray, and a set
            // of (possibly) stationary sample planes.
    
            // much of this should be more at home on the CPU or in a VS.
    
            // structured sampling pattern line normals
            float3 n0 = (abs( rd.x ) > abs( rd.z )) ? float3(1., 0., 0.) : float3(0., 0., 1.); // non diagonal
            float3 n1 = float3(mysign( rd.x * rd.z ), 0., 1.); // diagonal

            // normal lengths (used later)
            float2 ln = float2(length( n0 ), length( n1 ));
            n0 /= ln.x;
            n1 /= ln.y;

            // some useful DPs
            float2 ndotro = float2(dot( ro, n0 ), dot( ro, n1 ));
            float2 ndotrd = float2(dot( rd, n0 ), dot( rd, n1 ));

            // step size
            float2 period = ln * PERIOD;
            dt = period / abs( ndotrd );

            // dist to line through origin
            float2 dist = abs( ndotro / ndotrd );

            // raymarch start offset - skips leftover bit to get from ro to first strata lines
            t = -mysign( ndotrd ) * fmod( ndotro, period ) / abs( ndotrd );
            if( ndotrd.x > 0. ) t.x += dt.x;
            if( ndotrd.y > 0. ) t.y += dt.y;

            // sample weights
            float minperiod = PERIOD;
            float maxperiod = sqrt( 2. )*PERIOD;
            wt = smoothstep( maxperiod, minperiod, dt/ln );
            wt /= (wt.x + wt.y);
        }

        vec4 raymarch( in vec3 ro, in vec3 rd )
        {
            vec4 sum = vec4(0, 0, 0, 0);
    
            // setup sampling - compute intersection of ray with 2 sets of planes
            float2 t, dt, wt;
	        SetupSampling( t, dt, wt, ro, rd );
    
            // fade samples at far extent
            float f = .6; // magic number - TODO justify this
            float endFade = f*float(SAMPLE_COUNT)*PERIOD;
            float startFade = .8*endFade;
    
            for(int i=0; i<SAMPLE_COUNT; i++)
            {
                if( sum.a > 0.99 ) continue;

                // data for next sample
                vec4 data = t.x < t.y ? vec4( t.x, wt.x, dt.x, 0. ) : vec4( t.y, wt.y, 0., dt.y );
                // somewhat similar to: https://www.shadertoy.com/view/4dX3zl
                //vec4 data = mix( vec4( t.x, wt.x, dt.x, 0. ), vec4( t.y, wt.y, 0., dt.y ), float(t.x > t.y) );
                vec3 pos = ro + data.x * rd;
                float w = data.y;
                t += data.zw;
        
                // fade samples at far extent
                w *= smoothstep( endFade, startFade, data.x );
        
                vec4 col = map( pos );
        
                // iqs goodness
                float dif = clamp((col.w - map(pos+0.6*sundir).w)/0.6, 0.0, 1.0 );
                vec3 lin = vec3(0.51, 0.53, 0.63)*1.35 + 0.55*vec3(0.85, 0.57, 0.3)*dif;
                col.xyz *= lin;
        
                col.xyz *= col.xyz;
        
                col.a *= 0.75;
                col.rgb *= col.a;

                // integrate. doesn't account for dt yet, wip.
                sum += col * (1.0 - sum.a) * w;
            }

            sum.xyz /= (0.001+sum.w);

            return clamp( sum, 0.0, 1.0 );
        }

        vec3 sky( vec3 rd )
        {
            vec3 col = vec3(0.);
    
            float hort = 1. - clamp(abs(rd.y), 0., 1.);
            col += 0.5*vec3(.99,.5,.0)*exp2(hort*8.-8.);
            col += 0.1*vec3(.5,.9,1.)*exp2(hort*3.-3.);
            col += 0.55*vec3(.6,.6,.9);
    
            float sun = clamp( dot(sundir,rd), 0.0, 1.0 );
            col += .2*vec3(1.0,0.3,0.2)*pow( sun, 2.0 );
            col += .5*vec3(1.,.9,.9)*exp2(sun*650.-650.);
            col += .1*vec3(1.,1.,0.1)*exp2(sun*100.-100.);
            col += .3*vec3(1.,.7,0.)*exp2(sun*50.-50.);
            col += .5*vec3(1.,0.3,0.05)*exp2(sun*10.-10.); 
    
            float ax = atan(rd.y,length(rd.xz))/1.;
            float ay = atan(rd.z,rd.x)/2.;
            float st = 0.0f;
            float st2 = 0.0f;
            st *= st2;
            st = smoothstep(0.65,.9,st);
            col = mix(col,col+1.8*st,clamp(1.-1.1*length(col),0.,1.));
    
            return col;
        }

        void mainImage( out vec4 fragColor, in vec2 fragCoord )
        {
            // click mouse to use naive raymarching
            STRUCTURED = false;
            sundir = normalize(vec3(-1.0,0.0,-1.));

            vec2 q = fragCoord.xy / iResolution.xy;
            vec2 p = -1.0 + 2.0*q;
            p.x *= iResolution.x/ iResolution.y;
            vec2 mo = -1.0 + 2.0*iMouse.xy / iResolution.xy;
   
            // camera
            vec3 lookDir = vec3(cos(.53*iTime),0.,sin(iTime));
            vec3 camVel = vec3(-20.,0.,0.);

            vec3 ro = vec3(0.,1.5,0.) + iTime*camVel;
            vec3 ta = ro + lookDir; //vec3(ro.x, ro.y, ro.z-1.);
            vec3 ww = normalize( ta - ro);
            vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
            vec3 vv = normalize(cross(ww,uu));
            float fov = 1.;
            vec3 rd = normalize( fov*p.x*uu + fov*1.2*p.y*vv + 1.5*ww );
    
            // divide by forward component to get fixed z layout instead of fixed dist layout
            //vec3 rd_layout = rd/mix(dot(rd,ww),1.0,samplesCurvature);
            vec4 clouds = raymarch( ro, rd );
    
            vec3 col = clouds.xyz;
        
            // sky if visible
            if( clouds.w <= 0.99 )
	            col = mix( sky(rd), col, clouds.w );
    
	        col = clamp(col, 0., 1.);
            col = smoothstep(0.,1.,col);
	        col *= pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.12 ); //Vign
        
            fragColor = vec4( col, 1.0 );
        }

        void main(void)
        {
            mainImage(gl_FragColor, gl_FragCoord.xy);
        }
	)")
            )
            .end("GlslSandbox", QGlslSandboxRenderPass::Out::Output);

		addSection(0.5f, [this](const Context& ctx) {
			mAudioProvider->play();
            mWidget.setFrameGraph(mStage0);
		});

		addSection(2.5f, [this](const Context& ctx) {
			mWidget.setFrameGraph(mStage1);
		});

		//addSection(1.0f, 2.0f, 
		//	[](const Context& ctx) {
		//		qDebug() << ctx.currTimeMs << "B Start";
		//	},
		//	[](const Context& ctx) {
		//		qDebug() << ctx.currTimeMs << "B Tick";
		//	},
		//	[](const Context& ctx) {
		//		qDebug() << ctx.currTimeMs << "B End";
		//	}
		//);
		mWidget.showAndWaitInitialized();

        mStage0->compile(mWidget.getRenderer());
        mStage0->resize({ 852,900 });
		mStage1->compile(mWidget.getRenderer());
		mStage1->resize({ 852,900 });
	}
};

int main(int argc, char** argv) {
	QEngineApplication app(argc, argv);

	MySequence sequence;
	sequence.play();
	return app.exec();
}