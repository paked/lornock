typedef hmm_v2 vec2;
typedef hmm_v3 vec3;
typedef hmm_v4 vec4;
typedef hmm_m4 mat4;
typedef hmm_quaternion quat;

#define vec2(x, y)        HMM_Vec2(x, y)
#define vec3(x, y, z)     HMM_Vec3(x, y, z)
#define vec4(x, y, z, w)  HMM_Vec4(x, y, z, w)
#define mat4d(d) HMM_Mat4d(d)

#define quatToMat4(q) HMM_QuaternionToMat4(q)

#define quatFromAxisAngle(axis, angle) HMM_QuaternionFromAxisAngle(axis, angle)

#define mat4Perspective(fov, ratio, near, far) HMM_Perspective(fov, ratio, near, far)
#define mat4Orthographic(left, right, bottom, top, near, far) HMM_Orthographic(left, right, bottom, top, near, far)
#define mat4LookAt(m, eye, target) HMM_Multiply(m, HMM_LookAt(eye, target, vec3(0, 1, 0)))

#define mat4Translate(m, v) HMM_Multiply(m, HMM_Translate(v))
#define mat4Scale(m, v) HMM_Multiply(m, HMM_Scale(v))
#define mat4Rotate(m, a, v) HMM_Multiply(m, HMM_Rotate(a, v))

#define vec3Normalize(v) HMM_NormalizeVec3(v)
#define vec4Normalize(v) HMM_NormalizeVec4(v)
