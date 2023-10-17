vec3 correct_normals(mat4 model_matrix, vec3 normals) { return normalize(transpose(inverse(mat3(model_matrix))) * normals); }
vec3 correct_normals(mat4 model_matrix, vec4 normals) { return correct_normals(model_matrix, vec3(normals)); }

const mat4 _matrix = mat4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.5, 0.5, 0.0, 1.0);
mat4 bias_matrix() { return _matrix; }
