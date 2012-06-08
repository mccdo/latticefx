#version 120


//uniform vec4 fvAmbient;
//uniform vec4 fvSpecular;
//uniform vec4 fvDiffuse;
//uniform float fSpecularPower;

uniform sampler3D VolumeTexture;
uniform sampler2D TransferFunction;

varying vec3 Texcoord;
//varying vec3 ViewDirection;
//varying vec3 LightDirection;
//varying vec3 Normal;


void main( void )
{
/*
   vec3  fvLightDirection = normalize( LightDirection );
   vec3  fvNormal         = normalize( Normal );
   float fNDotL           = dot( fvNormal, fvLightDirection ); 
   
   vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection ); 
   vec3  fvViewDirection  = normalize( ViewDirection );
   float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );
*/ 
   vec3 modTexCoord = Texcoord;
   vec4  fvBaseColor = vec4(0.0, 0.0, 0.0, 0.0);
   if (Texcoord.x > 0.0 && Texcoord.x < 1.0 && Texcoord.y > 0.0 && Texcoord.y < 1.0 && Texcoord.z > 0.0 && Texcoord.z < 1.0)
   {
      fvBaseColor      = texture3D( VolumeTexture, modTexCoord );
      //fvBaseColor.a = fvBaseColor.r;
      //fvBaseColor.rgb      = texture2D( TransferFunction, vec2(fvBaseColor.r, 0.0) ).rgb;
      fvBaseColor      = texture2D( TransferFunction, vec2(fvBaseColor.r, 0.0) );
   }
   else
   {
      discard;
   }
   
   //vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 
   //vec4  fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
   //vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );
  
   float alphaThresh = 1.0/255.0;
   if (fvBaseColor.a < alphaThresh)
   {
     discard;
   }
   gl_FragColor = ( fvBaseColor );
       
}
