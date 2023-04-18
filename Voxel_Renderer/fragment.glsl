#version 330 core

out vec4 FragColor;
in vec2 fragPos;

uniform vec2 u_Resolution;

uniform float u_Time;

uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

struct Sphere {
	vec3 pos;
	float radius;
	vec3 color;
};

struct intersection {
	float t;
	vec3 pos;
	vec3 normal;
	vec3 color;
	bool hit;
};

const int numSpheres = 4;

Sphere spheres[numSpheres] = Sphere[](
	Sphere(vec3(-2, -2, 0), .9, vec3(1, 0.1, 0.1)),
	Sphere(vec3( -2, 2, 0), .9, vec3(0.1, 1, 0.1)),
	Sphere(vec3( 2, 2, 0), .9, vec3(0.1, 0.1, 1)),
	Sphere(vec3( 2, -2, 0), .9, vec3(0.1, 0.1, 1))
);

float sphereIntersect(Sphere sphere, vec3 pos, vec3 dir) {
	vec3 oc = pos - sphere.pos;
	float b = dot(oc, dir);
	float c = dot(oc, oc) - sphere.radius * sphere.radius;
	float h = b * b - c;
	if (h < 0) return -1;
	return -b - sqrt(h);
}

void main() {
	
	vec2 ScreenSpace = (gl_FragCoord.xy) / u_Resolution.xy;
	vec4 Clip = vec4(ScreenSpace.xy * 2.0f - 1.0f, -1.0, 1.0);
	vec4 Eye = vec4(vec2(u_InverseProjection * Clip), -1.0, 0.0);
	vec3 RayDirection = vec3(u_InverseView * Eye);
	vec3 RayOrigin = u_InverseView[3].xyz;
	RayDirection = normalize(RayDirection);
	
	vec3 col = vec3(0);

	intersection closest;
	closest.t = 1000000;

	for (int i = 0; i < numSpheres; i++) {
		float t = sphereIntersect(spheres[i], RayOrigin, RayDirection);
		if (t > 0 && t < closest.t) {
			closest.t = t;
			closest.pos = RayOrigin + RayDirection * t;
			closest.normal = normalize(closest.pos - spheres[i].pos);
			closest.color = spheres[i].color;
			closest.hit = true;
		}
	}

	if(closest.hit) {
		// basic phong shading
		vec3 lightDir = normalize(vec3(1, 1, -2));
		vec3 lightCol = vec3(1, 1, 1);
		vec3 specCol = vec3(1, 1, 1) * 4;
		float diff = max(dot(closest.normal, lightDir), 0.0);
		float spec = pow(max(dot(reflect(-lightDir, closest.normal), -RayDirection), 0.0), 64);

		col = closest.color * (diff * lightCol + spec * specCol);
	}

	FragColor = vec4(col, 1);
}
