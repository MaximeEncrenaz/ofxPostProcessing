/*
 *  NoiseWarpPass.cpp
 *
 *  Copyright (c) 2013, Neil Mendoza, http://www.neilmendoza.com
 *  All rights reserved. 
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met: 
 *  
 *  * Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 *  * Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *  * Neither the name of Neil Mendoza nor the names of its contributors may be used 
 *    to endorse or promote products derived from this software without 
 *    specific prior written permission. 
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE. 
 *
 */
#include "CustomWarp.h"

namespace itg
{
    CustomWarp::CustomWarp(const ofVec2f& aspect, bool arb, float frequency, float amplitude, float speed) :
        frequency(frequency), amplitude(amplitude), speed(speed), RenderPass(aspect, arb, "noisewarp")
    {
        m_People.clear();
        
        /*string vertShaderSrc = "#version 120\n";
        vertShaderSrc += STRINGIFY(
            
            uniform sampler2D tex;
            
            uniform float frequency;
            uniform float amplitude;
            uniform float time;
            uniform float speed;
            uniform int size;
            uniform float centroids[100];
            uniform float mouseRange;
            uniform vec2 mousePos;
            uniform vec4 mouseColor;
                                         
            void main()
            {
                float thresh = 150.0;
                // copy position so we can work with it.
                vec4 pos = gl_Vertex;
                
                // direction vector from mouse position to vertex position.
                vec2 dir = pos.xy - centroids[0];
                
                // distance between the mouse position and vertex position.
                float dist =  sqrt(dir.x * dir.x + dir.y * dir.y);
                
                // check vertex is within mouse range.
                if(dist > 0.0 && dist < thresh) {
                    
                    // normalise distance between 0 and 1.
                    float distNorm = dist / thresh;
                    
                    // flip it so the closer we are the greater the repulsion.
                    distNorm = 1.0 - distNorm;
                    
                    // make the direction vector magnitude fade out the further it gets from mouse position.
                    dir *= distNorm;
                    
                    // add the direction vector to the vertex position.
                    pos.x += dir.x;
                    pos.y += dir.y;
                }
                
                vec2 noise = vec2(0,0);
                
                for (int i=0; i< size; i++) {
                    vec2 controlPoint = vec2(centroids[2*i],centroids[2*i+1]);
                    float dampCoef = 1.0/10.0*exp(-distance( controlPoint,gl_TexCoord[0].st) / 0.05);
                    noise += vec2(
                                  dampCoef*cos(distance( controlPoint,gl_TexCoord[0].st) ),
                                  dampCoef*cos(distance( controlPoint,gl_TexCoord[0].st) )
                                  );
                }
                
                pos.x += noise.x;
                pos.y += noise.y;
                
                // finally set the pos to be that actual position rendered
                gl_TexCoord[0] =  pos;
            }
                                         
        );*/
        string fragShaderSrc = STRINGIFY(
            uniform sampler2D tex;

            uniform float frequency;
            uniform float amplitude;
            uniform float time;
            uniform float speed;
            uniform int size;
            uniform float centroids[100];
                                         
            void main()
            {
                vec2 noise = vec2(0,0);

                for (int i=0; i< size; i++) {
                    vec2 controlPoint = vec2(centroids[2*i],centroids[2*i+1]);
                    float dampCoef = 1.0/1.0*exp(-distance( controlPoint,gl_TexCoord[0].st) / 0.05);
                    noise += vec2(
                                 dampCoef*sin(distance( controlPoint,gl_TexCoord[0].st) ),
                                 dampCoef*sin(distance( controlPoint,gl_TexCoord[0].st) )
                                 );
                }
                
                vec2 texCoords = gl_TexCoord[0].st + noise;
                
                // mirror vertically to match reality
                //texCoords = vec2(texCoords.s, 1.0-texCoords.t);
                gl_FragColor = texture2D(tex, texCoords);
            }
        );
        
        //shader.setupShaderFromSource(GL_VERTEX_SHADER, vertShaderSrc);
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShaderSrc);
        shader.linkProgram();
    }
    
    void CustomWarp::render(ofFbo& readFbo, ofFbo& writeFbo, ofTexture& depth)
    {
        writeFbo.begin();
        shader.begin();
        shader.setUniform1f("time", ofGetElapsedTimef());
        shader.setUniformTexture("tex", readFbo.getTextureReference(), 0);
        shader.setUniform1f("frequency", frequency);
        shader.setUniform1f("amplitude", amplitude);
        shader.setUniform1f("speed", speed);
        shader.setUniform1i("size", m_People.size());
        float v[2*m_People.size()];
        for (int i=0; i < 2*m_People.size(); i+=2)
        {
            v[i] = m_People[i/2]->centroid.x;
            v[i+1] = m_People[i/2]->centroid.y;
            //shader.setUniform2f(ss.str().c_str(), m_People[i]->centroid.x, m_People[i]->centroid.y);
        }
        shader.setUniform1fv("centroids", v, 2*m_People.size());
        texturedQuad(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
        
        shader.end();
        writeFbo.end();
    }
} // namespace itg