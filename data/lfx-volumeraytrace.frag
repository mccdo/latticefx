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

    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    float specExp = 10.; // gl_FrontMaterial.shininess
    vec4 spec = gl_FrontLightProduct[0].specular *
        pow( max( dot( reflectVec, eyeVec ), 0. ), specExp );

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
    float rangeIndex = index;
    if( range > 0. )
        rangeIndex = ( index - tfRange.x ) / range;

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


// These two variables come from osgWorks, osgwTools library,
//   MultiCameraProjectionMatrix class, which allows the
//   OSG near & far computation to span two Cameras.
uniform mat4 osgw_ProjectionMatrix;
uniform mat4 osgw_ProjectionMatrixInverse;

/** start clipping */

uniform vec3 volumeDims;
uniform vec3 volumeCenter;

flat varying vec3 edgeEps, maxEps, invRes;

vec4 texToEyeCoords( in vec3 tc )
{
    vec3 sbTC = ( tc - edgeEps ) / ( maxEps - edgeEps );
    vec4 oc = vec4( ( sbTC - .5 ) * volumeDims + volumeCenter, 1. );
    return( gl_ModelViewMatrix * oc );
}

// Clip a ray (ok, a line segment, actually) with given endpoints.
// If both endpoints clipped, discard.
// If both endpoints unclipped, return unmodified values.
// If clipped, modify start or end accordingly.
void clipRay( inout vec3 start, inout vec3 end, in vec4 clipPlane )
{
    // Must clip in eye coord space. There is no practical way to clip in tex coord
    // space, which would be more desireable. The reason is that we'd have to back-
    // transform the eye coord clip plane into object coords as an interim step, and
    // that would require the inverse of the model matrix used to transform the clip
    // planes, which we don't have access to.

    float dotStart = dot( texToEyeCoords( start ), clipPlane );
    float dotEnd = dot( texToEyeCoords( end ), clipPlane );
    if( dotStart < 0. )
    {
        if( dotEnd < 0. )
            // both start and end were clipped.
            discard;

        // else, clip the start point.
        float pct = dotEnd / ( dotEnd - dotStart );
        start = ( start - end ) * pct + end;
    }
    else if( dotStart >= 0. )
    {
        if( dotEnd >= 0. )
            // both start and end are not clipped.
            return;

        // else, clip the end point.
        float pct = dotStart / ( dotStart - dotEnd );
        end = ( end - start ) * pct + start;
    }
}

uniform int volumeClipPlaneEnable0;
uniform int volumeClipPlaneEnable1;
uniform int volumeClipPlaneEnable2;
uniform int volumeClipPlaneEnable3;
uniform int volumeClipPlaneEnable4;
uniform int volumeClipPlaneEnable5;

// Return 0.0 if clipped.
// Return 1.0 if not clipped.
float clipping( in vec3 tc )
{
    vec4 ec = texToEyeCoords( tc );

    // Determine if inside the view. We really only care about the
    // front plane, so set inView=true if not clipped by front plane.
    vec4 cc = osgw_ProjectionMatrix * ec;
    bool inView = cc.z >= -cc.w;

    // Inside clip planes? Set inClipPlane=true if not clipped by any planes.
    bvec4 clipResultA = bvec4(
        ( volumeClipPlaneEnable0 > 0 ) ? ( dot( ec, gl_ClipPlane[ 0 ] ) >= 0. ) : true,
        ( volumeClipPlaneEnable1 > 0 ) ? ( dot( ec, gl_ClipPlane[ 1 ] ) >= 0. ) : true,
        ( volumeClipPlaneEnable2 > 0 ) ? ( dot( ec, gl_ClipPlane[ 2 ] ) >= 0. ) : true,
        ( volumeClipPlaneEnable3 > 0 ) ? ( dot( ec, gl_ClipPlane[ 3 ] ) >= 0. ) : true );
    bvec2 clipResultB = bvec2(
        ( volumeClipPlaneEnable4 > 0 ) ? ( dot( ec, gl_ClipPlane[ 4 ] ) >= 0. ) : true,
        ( volumeClipPlaneEnable5 > 0 ) ? ( dot( ec, gl_ClipPlane[ 5 ] ) >= 0. ) : true );
    return( float( inView && all( clipResultA ) && all( clipResultB ) ) );
}

/** end clipping */



uniform sampler3D VolumeTexture;

uniform float volumeTransparency;
uniform bool volumeTransparencyEnable;

uniform float volumeMaxSamples;

// These uniforms must be specified by the application. Lfx does not have access to them.
uniform vec2 windowSize;
uniform sampler2D sceneDepth;


flat varying vec3 tcEye;
flat varying float ecVolumeSize;

void main( void )
{
    // Compute window tex coords to sample the depth textures.
    vec2 winTC = gl_FragCoord.xy / windowSize;

    // Must interpolate tex coords along the ray, in theory, from the
    // eye (tcStart) to the current tex coord for this fragment (because
    // we are rendering the back faces of the volume cube).
    vec3 tcEnd = gl_TexCoord[0].xyz;
    vec3 tcStart = tcEye;


    // Tighten tcStart and tcEnd.
    // 
    // This shader will walk along the ray from tcStart to tcEnd.
    // For best performance, the code makes tcStart and tcEnd as
    // close to each other as possible, so we "tighten" tcStart and
    // tcEnd with the following three steps:
    // 1. Ensure tcStart is either in the volume, or clip it so that
    //    it liews on the volume boundary.
    // 2. Compare against the already rendered scene depth value.
    //    Discard this fargment if tcStart is behind it, or clip the
    //    ray if scene depth is within the endpoints.
    // 3. Clip the ray by any enabled model space clip planes.

    // Step 1:
    // If the ray start (tcStart) is outside the volume, clip
    // it to the volume boundaries.
    if( ( tcStart.x < edgeEps.x ) || ( tcStart.x > maxEps.x ) ||
        ( tcStart.y < edgeEps.y ) || ( tcStart.y > maxEps.y ) ||
        ( tcStart.z < edgeEps.z ) || ( tcStart.z > maxEps.z ) )
    {
        // Compute the ray start position such that it lies on a
        // volume face. The code below is using algebraic shorthand
        // to compute the ray/plane intersection point.
        if( tcStart.x < edgeEps.x )
        {
            vec3 ray = tcEnd - tcStart;
            if( ray.x != 0. )
            {
                float t = ( edgeEps.x - tcStart.x ) / ray.x;
                tcStart = tcStart + t * ray;
            }
            else
                tcStart.x = edgeEps.x;
        }
        if( tcStart.x > maxEps.x )
        {
            vec3 ray = tcEnd - tcStart;
            if( ray.x != 0. )
            {
                float t = ( maxEps.x - tcStart.x ) / ray.x;
                tcStart = tcStart + t * ray;
            }
            else
                tcStart.x = maxEps.x;
        }
        if( tcStart.y < edgeEps.y )
        {
            vec3 ray = tcEnd - tcStart;
            if( ray.y != 0. )
            {
                float t = ( edgeEps.y - tcStart.y ) / ray.y;
                tcStart = tcStart + t * ray;
            }
            else
                tcStart.y = edgeEps.y;
        }
        if( tcStart.y > maxEps.y )
        {
            vec3 ray = tcEnd - tcStart;
            if( ray.y != 0. )
            {
                float t = ( maxEps.y - tcStart.y ) / ray.y;
                tcStart = tcStart + t * ray;
            }
            else
                tcStart.y = maxEps.y;
        }
        if( tcStart.z < edgeEps.z )
        {
            vec3 ray = tcEnd - tcStart;
            if( ray.z != 0. )
            {
                float t = ( edgeEps.z - tcStart.z ) / ray.z;
                tcStart = tcStart + t * ray;
            }
            else
                tcStart.z = edgeEps.z;
        }
        if( tcStart.z > maxEps.z )
        {
            vec3 ray = tcEnd - tcStart;
            if( ray.z != 0. )
            {
                float t = ( maxEps.z - tcStart.z ) / ray.z;
                tcStart = tcStart + t * ray;
            }
            else
                tcStart.z = maxEps.z;
        }
    }

    // Step 2:
    // Tighten tcStart and tcEnd further by comparing against the
    // existing scene's depth value for this pixel.

    // Transform the scene depth value into tex coord space. We don't want
    // to shoot a ray past this point.
    float winZScene = texture2D( sceneDepth, winTC ).r;
    vec3 ndcScene = vec3( winTC, winZScene ) * 2. - 1.;
    vec4 ecScene = osgw_ProjectionMatrixInverse * vec4( ndcScene, 1. );
    ecScene /= ecScene.w;
    vec4 ocScene = gl_ModelViewMatrixInverse * ecScene;
    vec3 tcScene = ( ocScene.xyz - volumeCenter ) / volumeDims + vec3( .5 );
    tcScene = tcScene * ( 1. - invRes ) + edgeEps;
    // Compute the tex coord vector from the eye to tcScene.
    vec3 scenePlaneNormal = tcEye - tcScene;

    // Clip against the scene depth value
    if( dot( tcStart - tcScene, scenePlaneNormal ) < 0. )
        // Volume is behind the scene depth value. Do nothing.
        discard;
    if( dot( tcEnd - tcScene, scenePlaneNormal ) < 0. )
        // Stop the ray when it hits the scene depth value.
        tcEnd = tcScene;

    // Step 3:
    // Yet more tightening of tcStart and tcEnd, this time by
    // any enabled model space clip planes.

    if( volumeClipPlaneEnable0 > 0 )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 0 ] );
    if( volumeClipPlaneEnable1 > 0 )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 1 ] );
    if( volumeClipPlaneEnable2 > 0 )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 2 ] );
    if( volumeClipPlaneEnable3 > 0 )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 3 ] );
    if( volumeClipPlaneEnable4 > 0 )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 4 ] );
    if( volumeClipPlaneEnable5 > 0 )
        clipRay( tcStart, tcEnd, gl_ClipPlane[ 5 ] );


    // Prepare to step along the ray.

    // Compute number of samples. sampleVec is the ray in tex coord space.
    vec3 sampleVec = tcEnd - tcStart;
    // Corner-to-corner, sampleVec could be as long as sqrt(3), or 1.7321.
    // But we never want totalSamples > volumeMaxSamples, so normalize length to (0..1).
    float totalSamples = ( length( sampleVec ) / 1.7321 ) * volumeMaxSamples;

    // Ensure at least 1 sample.
    //totalSamples = max( totalSamples, 1. );
    // TBD Hack alert.
    // I am seeing some TDR timeout errors on Win7 unless we clamp totalSamples
    // to the maximum value. This should not be necessary, as the math to compute
    // totalSamples should never result in a value greater than volumeMaxSamples. Hm.
    totalSamples = clamp( totalSamples, 1., volumeMaxSamples );

    // Accumulate color samples into finalColor, initially contains no color.
    vec4 finalColor = vec4( 0., 0., 0., 0. );
    vec4 litColor = vec4( 0., 0., 0., 0. );

    // Tex coord delta used for normal computation.
    // Use half of the inverse volume resolution for best results.
    vec3 tcDelta = edgeEps;

    // Track the last volume sample value and last computed normal. We can avoid
    // recomputing the normal if the new sample value matches the old.
    float lastSample = -1.;

    bool lastPassHM = false;
    float sampleCount = 0.;
    while( sampleCount < totalSamples )
    {
        float sampleLen = sampleCount / totalSamples;
        sampleCount += 1.;

        // Obtain volume sample.
        vec3 coord = tcStart + sampleVec * sampleLen;
        vec4 baseColor = texture3D( VolumeTexture, coord );

        if( baseColor.r != lastSample )
        {
            // Obtain transfer function color and alpha values.
            vec4 color = transferFunction( baseColor.r );

            if( hardwareMask( coord, color ) )
            {
                // TBD This is a workaround to eliminate sampling errors. The
                // code ensures that the first sample that passes the hardware
                // mask is the *exact* value that would pass the mask.
                // TBD The code needs to be modified to be more general-purpose.
                if( !lastPassHM && ( lastSample > -1. ) )
                {
                    lastPassHM = true;

                    float len = ( hmParams[3] - lastSample ) / ( baseColor.r - lastSample );
                    sampleCount -= ( 2. - len );

                    sampleLen = sampleCount / totalSamples;
                    sampleCount += 1.;
                    coord = tcStart + sampleVec * sampleLen;
                    baseColor.r = hmParams[3];
                    color = transferFunction( baseColor.r );
                }

                // We have passed the hardware mask. Compute a normal
                // and light the fragment.

                // Compute texture coord offsets for normal gradient computation.
                vec3 tcNegX = coord + vec3( -tcDelta.x, 0., 0. );
                vec3 tcPosX = coord + vec3( tcDelta.x, 0., 0. );
                vec3 tcNegY = coord + vec3( 0., -tcDelta.y, 0. );
                vec3 tcPosY = coord + vec3( 0., tcDelta.y, 0. );
                vec3 tcNegZ = coord + vec3( 0., 0., -tcDelta.z );
                vec3 tcPosZ = coord + vec3( 0., 0., tcDelta.z );

                // Support for non-linear transfer function requires that each volume sample
                // be used in turn as an index into the transfer function. Only then can we
                // compute a correct normal for the resulting surface.
                // Note: Expensive.
                vec3 negVec = vec3( clipping( tcNegX ) * transferFunction( texture3D( VolumeTexture, tcNegX ).r ).a,
                    clipping( tcNegY ) * transferFunction( texture3D( VolumeTexture, tcNegY ).r ).a,
                    clipping( tcNegZ ) * transferFunction( texture3D( VolumeTexture, tcNegZ ).r ).a );
                vec3 posVec = vec3( clipping( tcPosX ) * transferFunction( texture3D( VolumeTexture, tcPosX ).r ).a,
                    clipping( tcPosY ) * transferFunction( texture3D( VolumeTexture, tcPosY ).r ).a,
                    clipping( tcPosZ ) * transferFunction( texture3D( VolumeTexture, tcPosZ ).r ).a );
                vec3 normal = normalize( gl_NormalMatrix * ( negVec - posVec ) );
                litColor = fragmentLighting( color, normal );
            }
            else
            {
                lastPassHM = false;
                litColor = vec4( 0. );
            }

            lastSample = baseColor.r;
        }

        if( litColor.a > 0. )
        {
            if( !volumeTransparencyEnable )
            {
                // Transparency is off, so max the alpha.
                finalColor = vec4( litColor.rgb, 1. );
            }
            else
            {
                // Scale alpha as requested for varying degrees of transparency.
                litColor.a *= volumeTransparency;

                // Front to back blending:
                //    dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
                //    dst.a   = dst.a   + (1 - dst.a) * src.a
                vec4 srcColor = vec4( litColor.rgb * litColor.a, litColor.a );
                finalColor = ( 1. - finalColor.a ) * srcColor + finalColor;
            }

            if( finalColor.a > .95 )
                // It's opaque enough. Break out of the loop.
                // And, we take this branch if transparency is disabled because
                // we forced the alpha to 1.0.
                break;
        }
    }


    // Wrap up.

    gl_FragData[0] = finalColor;

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
}
