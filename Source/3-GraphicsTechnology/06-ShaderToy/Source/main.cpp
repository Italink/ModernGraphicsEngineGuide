#include "QEngineApplication.h"
#include "QRenderWidget.h"
#include "Render/PassBuilder/QOutputPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QGlslSandboxPassBuilder.h"

class MyRenderer : public IRenderer {
private:
	QSharedPointer<QGlslSandboxPassBuilder> mGlslSandboxPass{ new QGlslSandboxPassBuilder };
    QByteArray mShaderCode;
public:
	MyRenderer()
		: IRenderer({ QRhi::Vulkan })
	{
        mShaderCode = R"(
            /*
             * Original shader from: https://www.shadertoy.com/view/mtBSWc
             */

            #ifdef GL_ES
            precision highp float;
            #endif

            // glslsandbox uniforms
            uniform float time;
            uniform vec2 resolution;

            // shadertoy emulation
            #define iTime time
            #define iResolution resolution

            // --------[ Original ShaderToy begins here ]---------- //
            //philip.bertani@gmail.com

            const float numOct  = 5. ;  //number of fbm octaves
            float focus = 0.;
            float focus2 = 0.;
            #define pi  3.14159265

            float random(vec2 p) {
                //a random modification of the one and only random() func
                return fract( sin( dot( p, vec2(12., 90.)))* 5e5 );
            }

            mat2 rot2(float an){float cc=cos(an),ss=sin(an); return mat2(cc,-ss,ss,cc);}

            //this is taken from Visions of Chaos shader "Sample Noise 2D 4.glsl"
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
		)";
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
        QGlslSandboxPassBuilder::Output glslOut
			= graphBuilder.addPassBuilder("GlslSandboxPass", mGlslSandboxPass)
                .setShaderCode(mShaderCode);

		QOutputPassBuilder::Output cout
			= graphBuilder.addPassBuilder<QOutputPassBuilder>("OutputPass")
			.setInitialTexture(glslOut.GlslSandboxResult);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QEngineApplication app(argc, argv);
	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}