#version 120


uniform sampler3D VolumeTexture;
uniform sampler2D TransferFunction;

varying vec3 Texcoord;
varying vec3 TexcoordUp;
varying vec3 TexcoordRight;
varying vec3 TexcoordBack;
varying vec3 TexcoordDown;
varying vec3 TexcoordLeft;
varying vec3 TexcoordFront;


/** begin light **/

varying vec3 ecVertex;

vec4 fragmentLighting( vec4 baseColor, vec3 normal )
{
    vec3 lightVec = normalize( gl_LightSource[0].position.xyz - ecVertex );
    vec3 eyeVec = normalize( -ecVertex );
    vec3 reflectVec = normalize( -reflect( lightVec, normal ) );

    vec4 amb = gl_LightSource[0].ambient * baseColor;

    vec4 diff = gl_LightSource[0].diffuse * baseColor * max( dot( normal, lightVec ), 0. );
    diff = clamp( diff, 0., 1. );
   
    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    float specExp = 10.; // gl_FrontMaterial.shininess
    vec4 spec = gl_FrontLightProduct[0].specular *
        pow( max( dot( reflectVec, eyeVec ), 0. ), specExp );
    spec = clamp( spec, 0., 1. );

    return( gl_FrontLightModelProduct.sceneColor + amb + diff + spec );
}

/** end light **/


bool TestInBounds(vec3 sample)
{
   return (sample.x > 0.0 && sample.x < 1.0 && sample.y > 0.0 && sample.y < 1.0 && sample.z > 0.0 && sample.z < 1.0);
}

void main( void )
{
   // turn transfer function off for testing with cone
   bool UseTransferFunc = false;
   vec4  fvBaseColor = vec4(0.0, 0.0, 0.0, 0.0);
   vec4  fvUpColor = vec4(0.0, 0.0, 0.0, 0.0);
   vec4  fvRightColor = vec4(0.0, 0.0, 0.0, 0.0);
   vec4  fvBackColor = vec4(0.0, 0.0, 0.0, 0.0);
   vec4  fvDownColor = vec4(0.0, 0.0, 0.0, 0.0);
   vec4  fvLeftColor = vec4(0.0, 0.0, 0.0, 0.0);
   vec4  fvFrontColor = vec4(0.0, 0.0, 0.0, 0.0);
   
   if (TestInBounds(Texcoord))
   {
      // Sample current fragment texture and six surrounding texture coordinates
      fvBaseColor    = texture3D( VolumeTexture, Texcoord );

      // apply transfer function to center sample first, to exploit early discard if possible
      if (! UseTransferFunc)
      {
        fvBaseColor.a  = fvBaseColor.r;
      }
      else
      {
        fvBaseColor    = texture2D( TransferFunction, vec2(fvBaseColor.r, 0.0) );
      }
      
      // try for early discard
      float alphaThresh = 1.0/255.0;
      if (fvBaseColor.a < alphaThresh)
      {
        //discard; // this seems to cause nothing to render. Odd.
      }

      if (TestInBounds(TexcoordUp))
      {
         fvUpColor      = texture3D( VolumeTexture, TexcoordUp );
      }
      if (TestInBounds(TexcoordRight))
      {
         fvRightColor   = texture3D( VolumeTexture, TexcoordRight );
      }
      if (TestInBounds(TexcoordBack))
      {
         fvBackColor    = texture3D( VolumeTexture, TexcoordBack );
      }
      if (TestInBounds(TexcoordDown))
      {
         fvDownColor    = texture3D( VolumeTexture, TexcoordDown );
      }
      if (TestInBounds(TexcoordLeft))
      {
         fvLeftColor   = texture3D( VolumeTexture, TexcoordLeft );
      }
      if (TestInBounds(TexcoordFront))
      {
         fvFrontColor    = texture3D( VolumeTexture, TexcoordFront );
      }
      
      // transfer all non-center samples
      if (! UseTransferFunc)
      {
        fvUpColor.a    = fvUpColor.r;
        fvRightColor.a = fvRightColor.r;
        fvBackColor.a  = fvBackColor.r;
        fvDownColor.a  = fvDownColor.r;
        fvLeftColor.a  = fvLeftColor.r;
        fvFrontColor.a = fvFrontColor.r;
      }
      else
      {
        fvUpColor      = texture2D( TransferFunction, vec2(fvUpColor.r, 0.0) );
        fvRightColor   = texture2D( TransferFunction, vec2(fvRightColor.r, 0.0) );
        fvBackColor    = texture2D( TransferFunction, vec2(fvBackColor.r, 0.0) );
        fvDownColor    = texture2D( TransferFunction, vec2(fvDownColor.r, 0.0) );
        fvLeftColor    = texture2D( TransferFunction, vec2(fvLeftColor.r, 0.0) );
        fvFrontColor   = texture2D( TransferFunction, vec2(fvFrontColor.r, 0.0) );
      }

      fvUpColor      = fvUpColor - fvBaseColor;
      fvRightColor   = fvRightColor - fvBaseColor;
      fvBackColor    = fvBackColor - fvBaseColor;
      fvDownColor    = fvDownColor - fvBaseColor;
      fvLeftColor    = fvLeftColor - fvBaseColor;
      fvFrontColor   = fvFrontColor - fvBaseColor;
      

      vec3 ecNormal = gl_NormalMatrix *
          vec3( fvLeftColor.a - fvRightColor.a, fvDownColor.a - fvUpColor.a, fvFrontColor.a - fvBackColor.a );

      float saveAlpha = fvBaseColor.a;
      fvBaseColor = fragmentLighting( fvBaseColor, ecNormal );
      fvBaseColor.a = saveAlpha;
   }
   else
   {
      discard;
   }
   
   gl_FragColor = ( fvBaseColor );
       
}
