#version 120
#extension GL_EXT_gpu_shader4 : enable


uniform float un[6];

// account for .000001 difference
float u_scene = round(un[5]);

#define WIDTH 1366
#define HEIGHT 768
#define AMBIENCE 300

float MAX_DIST;

float max3(vec3 v) {
	return max(max(v.x, v.y), v.z);
}

float opSub( float d1, float d2 ) {
    return max(-d1,d2);
}

float pMod1(inout float p, float size) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p + halfsize, size) - halfsize;
	return c;
}

float pModInterval1(inout float p, float size, float start, float stop) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p+halfsize, size) - halfsize;
	if (c > stop) {
		p += size*(c - stop);
		c = stop;
	}
	if (c <start) {
		p += size*(c - start);
		c = start;
	}
	return c;
}

vec2 fOpU(in vec2 obj0, in vec2 obj1) {
	return (obj0.x < obj1.x) ? obj0 : obj1;
}

float noise(in vec3 p)
{
    const vec3 s = vec3(7, 157, 113);
	vec3 ip = floor(p);
    vec4 h = vec4(0., s.yz, s.y + s.z) + dot(ip, s);
	p -= ip;
    p = p*p*(3. - 2.*p);
    h = mix(fract(sin(h)*43758.5453), fract(sin(h + s.x)*43758.5453), p.x);
    h.xy = mix(h.xz, h.yw, p.y);
    return mix(h.x, h.y, p.z);
}

mat3 m = mat3( 0.00,  1.60,  1.20, -1.60,  0.72, -0.96, -1.20, -0.96,  1.28 );

float fbm( vec3 p )
{
  float f = 0.5000*noise( p ); p = m*p*1.1;
  f += 0.2500*noise( p ); p = m*p*1.2;
  f += 0.1666*noise( p ); p = m*p;
  f += 0.0834*noise( p );
  return f;
}

mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0),
        vec3(-s, 0, c)
    );
}



float cubeSDF(vec3 p, vec3 b)
{
	return max3(abs(p) - b);
}

float sphereSDF(vec3 p, float r)
{
	return length(p) - r;
}

vec3 hash33(vec3 p)
{
    p = fract(p * vec3(443.8975,397.2973, 491.1871));
    p += dot(p.zxy, p.yxz+19.27);
    return fract(vec3(p.x * p.y, p.z*p.x, p.y*p.z));
}

vec3 stars(vec3 p)
{
    vec3 c = vec3(0.);
    float res = 70000;
    
	for (float i=0.;i<3;i++)
    {
        vec3 q = fract(p*(.15*res))-0.5;
        vec3 id = floor(p*(.15*res));
        vec2 rn = hash33(id).xy;
        float c2 = 1.-smoothstep(0.,.6,length(q));
        c2 *= step(rn.x,.0005+i*i*0.001);
        c += c2*(mix(vec3(1.0,0.49,0.1),vec3(0.75,0.9,1.),rn.y)*0.25+0.75);
        p *= 1.4;
    }
	return c*c*.7;
}

float cableSDF(vec3 p)
{
	return length(max(abs(p-vec3(cos(p.z*2)*.5,2+cos(p.z)*.3, 0))-vec3(.1,.02, 100),vec3(.0)));
}

float cylinderSDF( vec3 p, vec3 c )
{
	return length(p.xz-c.xy)-c.z;
}

vec3 gp;
vec2 map(vec3 p)
{
	// Rotation
	
	if(u_scene == 4)
		p *= rotateY(radians(90.0));
	
	gp = p;
	
	vec2 s;
	
	// Floating reflective-sphere-thingy
	
	vec3 p1 = vec3(p.x-13, p.y+1, p.z);
	pMod1(p1.x, 26);
	pMod1(p1.z, 24);
	pMod1(p1.y, 24);
	
	vec2 tr = vec2(cylinderSDF(p1, vec3(0., 0., 0.8)), 100.0);
	vec2 tr1 = vec2(cubeSDF(p1, vec3(2, 0.2, 20)), 4.0);
	
	vec2 fl = fOpU(fOpU(fOpU(vec2(sphereSDF(p1, 3.0), 3.0), vec2(cubeSDF(p1, vec3(6, 0.25, 6)), 4.0)), tr), tr1);
	
	// Cable
	
	vec2 cab = vec2(cableSDF(vec3(p1.x, p1.y, p1.z)), 5.0);
	
	if(mod(u_scene, 2.) == 0. || u_scene == 7.)
	{
		MAX_DIST = 280;
		
		pModInterval1(p.z, 18, 0, 40);
		
		// Ground plane
		
		vec2 gr = fOpU(
			vec2(
				opSub(
					cubeSDF(vec3(p.x, p.y, p.z), vec3(8, 02, 6)),
					cubeSDF(vec3(p.x, p.y, p.z), vec3(10.2, 0.2, 10))
				),
				1.0
			),
			vec2(cubeSDF(vec3(p.x, p.y+0.1, p.z), vec3(10, 0.2, 7)), 2.0)
		);
		
		// Floor / Ceiling
		vec2 bounds = fOpU(
			gr,
			vec2(cubeSDF(vec3(p.x, p.y-8, p.z), vec3(10.2, 0.2, 12)), 1.0)
		);
		
		bounds = fOpU(
			bounds,
			// Walls
			fOpU(
				vec2(cubeSDF(vec3(p.x+10, p.y-4, p.z), vec3(0.2, 4, 7.99)), 0),
				vec2(cubeSDF(vec3(p.x-10, p.y-4, p.z), vec3(0.2, 4, 7.99)), 0)
			)
		);
		
		pMod1(p.x, 20);
		pMod1(p.y, 10);
		
		s = bounds;

		// The part where the scenes merge together
		
		if(u_scene > 6) s = fOpU(s, fOpU(fl, cab));
	}else {
		MAX_DIST = 600;

		s = fOpU(fl, cab);
	}

	
	return s;
}

vec2 trace(vec3 ro, vec3 rd, float iter)
{
	float depth = 0;
	vec2 dist;
    for (int i = 0; i < iter; i++)
	{
        dist = map(ro + depth * rd);
        if (dist.x < 0.0001) {
			return vec2(depth, dist.y);
        }
        depth += dist.x*.5;
        if (depth >= MAX_DIST) {
            return vec2(MAX_DIST, 0);
        }
    }
	return vec2(MAX_DIST, 0);
}

vec3 getNormal( in vec3 p )
{
    return normalize(vec3(
        map(vec3(p.x + 0.0001, p.y, p.z)).x - map(vec3(p.x - 0.0001, p.y, p.z)).x,
        map(vec3(p.x, p.y + 0.0001, p.z)).x - map(vec3(p.x, p.y - 0.0001, p.z)).x,
        map(vec3(p.x, p.y, p.z  + 0.0001)).x - map(vec3(p.x, p.y, p.z - 0.0001)).x
    ));
}

vec3 doColor(in vec3 sp, in vec3 rd, in vec3 sn, in vec3 lp, vec3 base)
{
	vec3 ld = lp-sp;
	float distlpsp = max(length(ld), 0.001);
	ld /= distlpsp;
	vec3 color;
    float r = 0.0, w = 1.0, d;
    for (float i = 1.0; i < 5.1; i++)
	{
        d = i/5;
        r += w*(d - map(sp + sn*d).x);
        w *= 0.5;
    }
    float ao = 1.0-clamp(r,0.0,1.0);
	vec3 sunDir = vec3(0.1, 0.1, 0.9);
	//float sun = clamp( dot( sn, sunDir ), 0.0, 1.0 );
	//float ind = clamp( dot( sn, normalize(sunDir*vec3(-1.0,0.0,-1.0)) ), 0.0, 1.0 );
	vec3 li = clamp( 0.5 + 0.5 * sn.y, 0.0, 1.0 )*vec3(0.16,0.20,0.28)*ao;//+ind;
	color = li * base + (pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 8.)*color*.4);
	color *= pow( clamp(dot(sn, rd) + 1., .0, 1.), 1.)*max( dot(sn, ld), 0.0)*min(1./(distlpsp), 1)*AMBIENCE;
	
	return color;
}

void main()
{
	vec2 uv = (gl_FragCoord.xy - vec2(WIDTH, HEIGHT)*.5) / HEIGHT;
    vec3 rd = normalize(vec3(uv, 1.0));
    vec3 ro = vec3(un[2], un[3], un[4]);
	vec3 lp = vec3(ro.x, ro.y, ro.z);
	
    vec2 t = trace(ro, rd, 1400);
	
	if (t.x > MAX_DIST - 0.0001)
	{
		gl_FragColor = vec4(stars(rd)-un[1], 0.0);
		return;
	}

	ro += rd*t.x;
    vec3 sn = getNormal(ro);
	
	vec3 color;
	float shiny;
	if(t.y == 0.0)
	{
		// Wall material
		
		//vec3 c1 = vec3(cos(sin(un[0]))*2, 0.0, clamp(sin(un[0])*.2, 0.0, 1.0));
		vec3 c1 = vec3(0.4, 0.1, 0.2);
		vec3 c2 = vec3(0.6);
		
		color = mix(
			c1,
			c2,
			step(
				1.,
				abs(gp.y + 0.75 * -5.0
				+ sin(gp.z * 0.20 + (u_scene > 0 ? un[0] : 0) * .8) - fbm(gp * 1.5))
			)
		);
		
		shiny = 0.1;
	}else if(t.y == 1.0) {
		color = vec3(0.2, 0.2, 0.2);
		shiny = 0.6;
	}else if(t.y == 2.0) {
		// Floor material
		color = vec3(0.1, 0.3, 0.2);
		shiny = 0.09;
	}else if(t.y == 3.0) {
		// Sphere material, should be roughly synced to lead instrument
		float r = (u_scene >= 3 && u_scene != 8) ? un[0] : 0;
		color = vec3(0.1, clamp(sin(r), 0.0, 0.7), 0.2);
		shiny = 2.5;
	}else if(t.y == 4.0) {
		color = vec3(0.01);
		shiny = 1;
	}else if(t.y == 5.0) {
		// Cable material
		color = vec3(0.1, 0.2, 0.2);
		shiny = 0;
	}
	
	float fog = smoothstep(0., 1.1, t.x/MAX_DIST);
    vec3 sceneColor = doColor(ro, rd, sn, lp, color);
	
	// Reflection //
    
    rd = reflect(rd, sn);
    t = trace(ro +  rd*.01, rd, 750);
    ro += rd*t.x;
    sn = getNormal(ro);
    sceneColor += doColor(ro, rd, sn, lp, color)*shiny;
   
	// Post processing //
  
	// Scanlines
	sceneColor -= sin(uv.y*1100)*0.007;
	// Fog
	sceneColor = mix(sceneColor, vec3(0), fog);
	// Grain
	sceneColor = mix(sceneColor, vec3(fract(sin(dot(uv.xy, vec2(17.0,180.)+un[0]))* 2500)), 0.002);
	
	gl_FragColor = vec4(sqrt(clamp(sceneColor, 0.0, 1.0))-un[1], 1.0);
}
