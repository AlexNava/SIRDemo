// Fragment shader for the SIRD effect
// Copyright 2008 by Alessandro Nava
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA


varying vec2 depthTextureCoord;
varying vec2 noiseTextureCoord;
uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;
uniform float nearPlane;
uniform float farPlane;

const float shiftScale=0.25;
const float one_over_seven=(1.0/7.0);

float linearize(in float z){
    return((farPlane-nearPlane*farPlane/(farPlane-z*(farPlane-nearPlane)))/(farPlane-nearPlane));
//    return((3.0-0.1 * 3.0 / (3.0 - z * 2.9))/2.9);
//    return((gl_DepthRange.far-gl_DepthRange.near * gl_DepthRange.far / (gl_DepthRange.far - z * (gl_DepthRange.far-gl_DepthRange.near)))/(gl_DepthRange.far-gl_DepthRange.near));
}

void main(void){
    float height,extraheight;
    vec3 color;

	height=linearize(texture2D(depthTexture, depthTextureCoord).r);

    if((noiseTextureCoord.s+height*shiftScale)<1.0){
        color = texture2D(noiseTexture,noiseTextureCoord+height*vec2(shiftScale,0.0)).rgb;
    }
    else{
		extraheight=linearize(texture2D(depthTexture, (depthTextureCoord+vec2((height*shiftScale-1.0)*one_over_seven,0.0))).r);

		color = texture2D(noiseTexture,vec2(-1.0,0.0)+noiseTextureCoord+(height+extraheight)*vec2(shiftScale,0.0)).rgb;
// debug - draw the pixels that require a second pass in a slightly red shade
//        color += vec3(0.5,0.0,0.0);
    }
    gl_FragColor = vec4(color, 1.0);
// debug - plot the depth instead of the distorted texture
//    gl_FragColor = vec4(height, height, height, 1.0);
}
