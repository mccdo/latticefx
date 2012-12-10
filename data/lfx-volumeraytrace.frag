#version 120


/** begin light **/

varying vec3 ecVertex;

// 'normal' must be normalized.
vec4 fragmentLighting( vec4 baseColor, vec3 normal )
{
    vec3 lightVec = normalize( gl_LightSource[0].position.xyz - ecVertex );
    vec3 eyeVec = normalize( -ecVertex );
    vec3 reflectVec = -reflect( lightVec, normal ); // lightVec and normal are already normalized,

    vec4 amb = gl_LightSource[0].ambient * baseColor;

    vec4 diff = gl_LightSource[0].diffuse * baseColor * max( dot( normal, lightVec ), 0. );
    // TBD we should not need to clamp, and this should be removed from
    // all shader lighting code and then tested.
    //diff = clamp( diff, 0., 1. );
   
    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    float specExp = 10.; // gl_FrontMaterial.shininess
    vec4 spec = gl_FrontLightProduct[0].specular *
        pow( max( dot( reflectVec, eyeVec ), 0. ), specExp );
    // TBD we should not need to clamp, and this should be removed from
    // all shader lighting code and then tested.
    //spec = clamp( spec, 0., 1. );

    vec4 outColor = gl_FrontLightModelProduct.sceneColor + amb + diff + spec;
    outColor.a = baseColor.a;
    return( outColor );
}

/** end light **/


/** begin transfer function **/

uniform sampler1D tf1d;
uniform vec2 tfRange;
uniform int tfDimension;
uniform vec4 tfDest;

vec4 transferFunction( in float index )
{
    float range = tfRange.y - tfRange.x;
    float rangeIndex = ( index - tfRange.x ) / range;

    // Support only 1D transfer function for now.
    vec4 xfer = texture1D( tf1d, rangeIndex );

    // If tfDimension is non-zero, we get the normal destination mask.
    // If zero, set dest mask to all zeros to get all gl_Color.
    vec4 localDestMask = tfDest * min( float( tfDimension ), 1. );

    // localDestMask is rgba floats, and will be either 1.0 or 0.0 for each element.
    // For element=1.0, take element from the xfer function.
    // Otherwise, take element from glColor.
    vec4 returnColor = ( xfer * localDestMask )
        + ( gl_Color * ( 1. - localDestMask ) );

    return( returnColor );
}

/** end transfer function **/


/** begin hardware mask **/

uniform sampler3D hmInput;
uniform vec4 hmParams;
const float hmAlpha = 0.;
const float hmRed = 1.;
const float hmScalar = 2.;

// Return true if passed, false if failed.
bool hardwareMask( in vec3 tC, in vec4 baseColor )
{
    // hmParams has all mask parameters in a single vec4 uniform:
    //   Element 0: Input source (0=alpha, 1=red, 2=scalar, 1000=no mask
    //   Element 1: Mask operator (0=EQ, -1=LT, 1=GT).
    //   Element 2: Operator negate flag (1=no negate, -1=negate).
    //   Element 3: Reference value.

    float value;
    if( hmParams[ 0 ] < hmScalar )
    {
        // either red (1) or alpha (0).
        value = dot( baseColor.ra, vec2( hmParams[ 0 ], 1. - hmParams[ 0 ] ) );
    }
    else if( hmParams[ 0 ] == hmScalar )
    {
        // hmInput texture format is GL_ALPHA32F_ARB.
        value = texture3D( hmInput, tC ).a;
    }
    else
    {
        // no mask
        return( true );
    }

    // sign() returns 1=pos, 0=0, and -1=neg -- same as what we have in hmParams[ 1 ].
    // So if sign(value-ref) == hmParams[1], we have passed the hm test.
    bool result = ( sign( value - hmParams[ 3 ] ) == hmParams[ 1 ] );

    if( hmParams[ 2 ] == -1. ) // Negate
        result = !result;
    return( result );
}

/** end hardware mask **/



uniform float volumeMaxSamples;

uniform sampler3D VolumeTexture;

// These uniforms must be specified by the application. Lfx does not have access to them.
uniform vec2 windowSize;
uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;

varying vec3 Texcoord;
varying vec3 TexcoordUp;
varying vec3 TexcoordRight;
varying vec3 TexcoordBack;
varying vec3 TexcoordDown;
varying vec3 TexcoordLeft;
varying vec3 TexcoordFront;

varying vec3 ecUp;
varying vec3 ecRight;
varying vec3 ecBack;
varying vec3 ecDown;
varying vec3 ecLeft;
varying vec3 ecFront;


bool TestInBounds(vec3 sample)
{
   return (sample.x > 0.0 && sample.x < 1.0 && sample.y > 0.0 && sample.y < 1.0 && sample.z > 0.0 && sample.z < 1.0);
}


uniform vec3 volumeDims;
uniform vec3 volumeCenter;

vec4 texToEyeCoords( in vec3 tc )
{
    vec4 oc = vec4( ( tc - .5f ) * volumeDims + volumeCenter, 1.f );
    return( gl_ModelViewMatrix * oc );
}

void clipRay( inout vec3 start, inout vec3 end, in vec4 clipPlane )
{
    // Must clip in eye coord space. There is no practical way to clip in tex coord
    // space, which would be more desireable. The reason is that we'd have to back-
    // transform the eye coord clip plane into object coords as an interim step, and
    // that would require the inverse of the model matrix used to transform the clip
    // planes, which we don't have access to.

    float dotStart = dot( texToEyeCoords( start ), clipPlane );
    float dotEnd = dot( texToEyeCoords( end ), clipPlane );
    if( dotStart < 0.f )
    {
        if( dotEnd < 0.f )
            // both start and end were clipped.
            discard;

        // else, clip the start point.
        float pct = dotEnd / ( dotEnd - dotStart );
        start = ( start - end ) * pct + end;
    }
    else if( dotStart >= 0.f )
    {
        if( dotEnd >= 0.f )
            // both start and end are not clipped.
            return;

        // else, clip the end point.
        float pct = dotStart / ( dotStart - dotEnd );
        end = ( end - start ) * pct + start;
    }
}

uniform vec4 volumeClipPlaneEnables;

// Return 0.0 if clipped.
// Return 1.0 if not clipped.
float clipping( in vec3 ec )
{
    // Determine if inside the view. We really only care about the
    // front plane, so set inView=true if not clipped by front plane.
    vec4 cc = gl_ProjectionMatrix * vec4( ec, 1. );
    // step(a,b) = 1.0 if b>=a, 0.0 otherwise.
    bool inView = cc.z >= -cc.w;

    // Inside clip planes? Set inClipPlane=true if not clipped by any planes.
    vec4 ec4 = vec4( ec, 1. );
#if 1
    // Only support one plane for now for performance reasons.
    bool inClipPlane = (volumeClipPlaneEnables.x > 0.) ? ( dot( ec4, gl_ClipPlane[ 0 ] ) >= 0. ) : true;

    // Return 1.0 if not plassed all the above clip tests.
    // Return 0.0 if one or more of the about tests failed.
    return( float( inView && inClipPlane ) );
#else
    bvec4 clipResult0 = bvec4(
        ( volumeClipPlaneEnables.x > 0. ) ? ( dot( ec4, gl_ClipPlane[ 0 ] ) >= 0. ) : true,
        ( volumeClipPlaneEnables.y > 0. ) ? ( dot( ec4, gl_ClipPlane[ 1 ] ) >= 0. ) : true,
        ( volumeClipPlaneEnables.z > 0. ) ? ( dot( ec4, gl_ClipPlane[ 2 ] ) >= 0. ) : true,
        ( volumeClipPlaneEnables.w > 0. ) ? ( dot( ec4, gl_ClipPlane[ 3 ] ) >= 0. ) : true );
    return( float( inView && all( clipResult0 ) ) );
#endif
}


varying vec3 tcEye;
varying float ecVolumeSize;

void main( void )
{
    // Compute window tex coords to sample the scene color and depth textures.
    vec2 winTC = gl_FragCoord.xy / windowSize;

    // Transform the scene depth value into tex coord space. We don't want
    // to shoot a ray past this point.
    float winZScene = texture2D( sceneDepth, winTC ).r;
    vec3 ndcScene = vec3( winTC, winZScene ) * 2.f - 1.f;
    vec4 ccScene = gl_ProjectionMatrixInverse * vec4( ndcScene, 1.f );
    ccScene /= ccScene.w;
    vec4 ocScene = gl_ModelViewMatrixInverse * ccScene;
    vec3 tcScene = ( ocScene.xyz - volumeCenter ) / volumeDims + vec3( .5 );
    // Compute the tex coord vector from the eye to tcScene.
    vec3 scenePlaneNormal = tcEye - tcScene;

    // Must interpolate tex coords along the ray.
    vec3 tcEnd = gl_TexCoord[0].xyz;
    vec3 tcStart = tcEye;

    if( ( tcStart.x >= 0. ) && ( tcStart.x <= 1. ) &&
        ( tcStart.y >= 0. ) && ( tcStart.y <= 1. ) &&
        ( tcStart.z >= 0. ) && ( tcStart.z <= 1. ) )
    {
        // tcStart is inside volume
    }
    else
    {
        // Compute the ray start position such that it lies on a
        // volume face. The code below is using algebraic shorthand
        // to compute the ray/plane intersection point.
        if( tcStart.x < 0. )
        {
            float t = -tcStart.x / ( tcEnd.x - tcStart.x );
            tcStart = tcStart + t * ( tcEnd - tcStart );
        }
        if( tcStart.x > 1. )
        {
            float t = ( 1. - tcStart.x ) / ( tcEnd.x - tcStart.x );
            tcStart = tcStart + t * ( tcEnd - tcStart );
        }
        if( tcStart.y < 0. )
        {
            float t = -tcStart.y / ( tcEnd.y - tcStart.y );
            tcStart = tcStart + t * ( tcEnd - tcStart );
        }
        if( tcStart.y > 1. )
        {
            float t = ( 1. - tcStart.y ) / ( tcEnd.y - tcStart.y );
            tcStart = tcStart + t * ( tcEnd - tcStart );
        }
        if( tcStart.z < 0. )
        {
            float t = -tcStart.z / ( tcEnd.z - tcStart.z );
            tcStart = tcStart + t * ( tcEnd - tcStart );
        }
        if( tcStart.z > 1. )
        {
            float t = ( 1. - tcStart.z ) / ( tcEnd.z - tcStart.z );
            tcStart = tcStart + t * ( tcEnd - tcStart );
        }
    }

    // Clip against the scene depth value
    if( dot( tcStart - tcScene, scenePlaneNormal ) < 0.f )
    {
        // Volume is behind the scene depth value. Do nothing.
        discard;
    }
    if( dot( tcEnd - tcScene, scenePlaneNormal ) < 0.f )
    {
        // Make the ray end when it hits the scene depth value.
        tcEnd = tcScene;
    }

    // Clip ray endpoints against enabled clip planes.
    if( volumeClipPlaneEnables.x > 0.f )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 0 ] );
    if( volumeClipPlaneEnables.y > 0.f )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 1 ] );
    if( volumeClipPlaneEnables.z > 0.f )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 2 ] );
    if( volumeClipPlaneEnables.w > 0.f )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 3 ] );

    vec3 sampleVec = tcEnd - tcStart;
    float sampleStepSize = ecVolumeSize / volumeMaxSamples;
    float totalSamples = ecVolumeSize * length( sampleVec ) / sampleStepSize;

    // Ensure a minimum totalSamples to reduce banding artifacts in thin areas.
    // I am seeing some TDR timeout errors on Win7 unless we clamp totalSamples
    // to some maximum value. This should not be necessary.
    totalSamples = clamp( totalSamples, 3.f, 1024.f );

    // Get the initial color from the rendered scene.
    vec4 finalColor = vec4( 0., 0., 0., 0. );

    float sample = 0.f;
    while( ++sample < totalSamples )
    {
        float sampleLen = sample / totalSamples;

        vec3 coord = tcStart + sampleVec * sampleLen;
        vec4 baseColor = texture3D( VolumeTexture, coord );

        vec4 color = transferFunction( baseColor.r );
        if( hardwareMask( coord, color ) )
        {
            // Front to back blending:
            //    dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
            //    dst.a   = dst.a   + (1 - dst.a) * src.a
            color.rgb *= color.a;
            finalColor = ( 1.f - finalColor.a ) * color + finalColor;

            if( finalColor.a > .95f )
                // It's opaque enough
                break;
        }
    }
    vec4 color = texture2D( sceneColor, winTC );
    color.rgb *= color.a;
    finalColor = ( 1.f - finalColor.a ) * color + finalColor;

    gl_FragData[0] = vec4( finalColor.rgb, 1. );

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
    return;

#if 0
    // Vectex shader always sends (eye oriented) quads. Much of the quad
    // might be outside the volume. Immediately discard if this is the case.
    if( !( TestInBounds( Texcoord ) ) )
        discard;

    // Get volume sample. Format is GL_KUMINANCE, so the same volume value
    // will be stored in fvBaseColor.rayLen, g, and b. fvBaseColor.a will be 1.0.
    vec4 fvBaseColor = texture3D( VolumeTexture, Texcoord );


    vec4 color = transferFunction( fvBaseColor.rayLen );
    if( !hardwareMask( Texcoord, color ) )
        discard;


#if 0
    // Support for non-linear transfer function requires that each volume sample
    // be used in turn as an index into the transfer function. Only then can we
    // compute a correct normal for the resulting surface.
    // Note: Expensive.
    vec3 frontVec = vec3( clipping( ecLeft ) * transferFunction( texture3D( VolumeTexture, TexcoordLeft ).rayLen ).a,
        clipping( ecDown ) * transferFunction( texture3D( VolumeTexture, TexcoordDown ).rayLen ).a,
        clipping( ecFront ) * transferFunction( texture3D( VolumeTexture, TexcoordFront ).rayLen ).a );
    vec3 backVec = vec3( clipping( ecRight ) * transferFunction( texture3D( VolumeTexture, TexcoordRight ).rayLen ).a,
        clipping( ecUp ) * transferFunction( texture3D( VolumeTexture, TexcoordUp ).rayLen ).a,
        clipping( ecBack ) * transferFunction( texture3D( VolumeTexture, TexcoordBack ).rayLen ).a );
#else

    // For performance reasons, do not use transfer function. This means the transfer
    // function alpha is a direct map with volume samples.
    vec3 frontVec = vec3( clipping( ecLeft ) * texture3D( VolumeTexture, TexcoordLeft ).rayLen,
        clipping( ecDown ) * texture3D( VolumeTexture, TexcoordDown ).rayLen,
        clipping( ecFront ) * texture3D( VolumeTexture, TexcoordFront ).rayLen );
#define USE_FAST_NORMAL_COMPUTATION
#ifdef USE_FAST_NORMAL_COMPUTATION
    vec3 backVec = clipping( ecVertex ) * fvBaseColor.rgb;
#else
    vec3 backVec = vec3( clipping( ecRight ) * texture3D( VolumeTexture, TexcoordRight ).rayLen,
        clipping( ecUp ) * texture3D( VolumeTexture, TexcoordUp ).rayLen,
        clipping( ecBack ) * texture3D( VolumeTexture, TexcoordBack ).rayLen );
#endif

#endif
    vec3 ecNormal = normalize( gl_NormalMatrix * ( frontVec - backVec ) );

    vec4 finalColor = fragmentLighting( color, ecNormal );


    gl_FragData[ 0 ] = finalColor;

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
#endif
}
