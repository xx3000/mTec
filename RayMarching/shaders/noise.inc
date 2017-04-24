/*
Copyright (c) <2015> <Playdead> L.Pedersen, MIT Licence

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


//Noise functions used by TAA

//note: normalized random, float=[0;1[
float PDnrand( vec2 n ) {
	return fract( sin(dot(n.xy, vec2(12.9898f, 78.233f)))* 43758.5453f );
}
vec2 PDnrand2( vec2 n ) {
	return fract( sin(dot(n.xy, vec2(12.9898f, 78.233f)))* vec2(43758.5453f, 28001.8384f) );
}
vec3 PDnrand3( vec2 n ) {
	return fract( sin(dot(n.xy, vec2(12.9898f, 78.233f)))* vec3(43758.5453f, 28001.8384f, 50849.4141f ) );
}
vec4 PDnrand4( vec2 n ) {
	return fract( sin(dot(n.xy, vec2(12.9898f, 78.233f)))* vec4(43758.5453f, 28001.8384f, 50849.4141f, 12996.89f) );
}

//====
//note: signed random, float=[-1;1[
float PDsrand( vec2 n ) {
	return PDnrand( n ) * 2 - 1;
}
vec2 PDsrand2( vec2 n ) {
	return PDnrand2( n ) * 2 - 1;
}
vec3 PDsrand3( vec2 n ) {
	return PDnrand3( n ) * 2 - 1;
}
vec4 PDsrand4( vec2 n ) {
	return PDnrand4( n ) * 2 - 1;
}
