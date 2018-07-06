typedef hmm_v2 vec2;
typedef hmm_v3 vec3;
typedef hmm_v4 vec4;
typedef hmm_m4 mat4;
typedef hmm_quaternion quat;

#define lerp(t, a, b) HMM_Lerp(a, t, b)

#define vec2(x, y)        HMM_Vec2(x, y)
#define vec3(x, y, z)     HMM_Vec3(x, y, z)
#define vec4(x, y, z, w)  HMM_Vec4(x, y, z, w)
#define mat4d(d) HMM_Mat4d(d)

#define quatNormalize(q) HMM_NormalizeQuaternion(q)
#define quatToMat4(q) HMM_QuaternionToMat4(q)
#define quatFromAxisAngle(axis, angle) HMM_QuaternionFromAxisAngle(axis, angle)
#define quatSlerp(l, t, r) HMM_Slerp(l, t, r)

#define mat4Perspective(fov, ratio, near, far) HMM_Perspective(fov, ratio, near, far)
#define mat4Orthographic(left, right, bottom, top, near, far) HMM_Orthographic(left, right, bottom, top, near, far)
#define mat4LookAt(m, eye, target) HMM_Multiply(m, HMM_LookAt(eye, target, vec3(0, 1, 0)))

#define mat4Translate(m, v) HMM_Multiply(m, HMM_Translate(v))
#define mat4Scale(m, v) HMM_Multiply(m, HMM_Scale(v))
#define mat4Rotate(m, a, v) HMM_Multiply(m, HMM_Rotate(a, v))

#define vec3Normalize(v) HMM_NormalizeVec3(v)
#define vec4Normalize(v) HMM_NormalizeVec4(v)

vec3 vec3Lerp(real32 t, vec3 a, vec3 b) {
  vec3 p;

  p.x = lerp(t, a.x, b.x);
  p.y = lerp(t, a.y, b.y);
  p.z = lerp(t, a.z, b.z);

  return p;
}

quat quatFromPitchYawRoll(real64 roll, real64 pitch, real64 yaw) {
	quat q = {0};

	// Abbreviations for the various angular functions
	real64 cy = cos(yaw * 0.5);
	real64 sy = sin(yaw * 0.5);
	real64 cr = cos(roll * 0.5);
	real64 sr = sin(roll * 0.5);
	real64 cp = cos(pitch * 0.5);
	real64 sp = sin(pitch * 0.5);

	q.W = cy * cr * cp + sy * sr * sp;
	q.X = cy * sr * cp - sy * cr * sp;
	q.Y = cy * cr * sp + sy * sr * cp;
	q.Z = sy * cr * cp - cy * sr * sp;

	return q;
}
