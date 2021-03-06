#define PI  3.1415926535897
#define PIf 3.1415926f

#define rad2Deg(a) ((a) * (180.0f/PIf))
#define deg2Rad(a) ((a) * (PIf/180.0f))

#define vec3FromVec4(v) (vec3(v.x, v.y, v.z))
#define vec4FromVec3(v) (vec4(v.x, v.y, v.z, 0.0f))

#define vec3_one (vec3(1, 1, 1))
#define vec3_zero (vec3(0, 0, 0))
#define vec3_up (vec3(0, 1, 0))
#define vec3_right (vec3(1, 0, 0))
#define vec3_forward (vec3(0, 0, -1.0))

#define vec3_white (vec3(255, 255, 255))

#define vec3_print(v, name) { logfln( #name ": x = %f, y = %f, z = %f", v.x, v.y, v.z); }

inline real32 rand01() {
  return (real32)rand() / (real32)RAND_MAX;
}

inline real32 randFromTo(real32 min, real32 max) {
  real32 amount = max - min;

  return min + rand01() * amount;
}

real32 vec3Sum(vec3 v) {
  return (v.x + v.y + v.z);
}

bool vec3AlmostEqual(vec3 a, vec3 b, real32 e=0.1) {
  return fabs(b.x - a.x) < e &&
    fabs(b.y - a.y) < e &&
    fabs(b.z - a.z) < e;
}

vec3 vec3Lerp(real32 t, vec3 a, vec3 b) {
  vec3 p;

  p.x = lerp(t, a.x, b.x);
  p.y = lerp(t, a.y, b.y);
  p.z = lerp(t, a.z, b.z);

  return p;
}

quat quatFromPitchYawRoll(real64 roll, real64 pitch, real64 yaw) {
	quat q = {0};

  roll = deg2Rad(roll);
  pitch = deg2Rad(pitch);
  yaw = deg2Rad(yaw);

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

real32 clamp(real32 v, real32 min, real32 max) {
  if (v < min) {
    return min;
  } else if (v > max) {
    return max;
  }

  return v;
}

struct Rect {
  real32 x, y;
  real32 w, h;
};

Rect rect_init(real32 x, real32 y, real32 w, real32 h) {
  Rect r;

  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;

  return r;
}
