vec3 correct_normals(mat4 model_matrix, vec3 normals){ return normalize(transpose(inverse(mat3(model_matrix))) * normals); }
vec3 correct_normals(mat4 model_matrix, vec4 normals){ return correct_normals(model_matrix, vec3(normals)); }

mat4 bias_matrix() {
    const mat4 bias_matrix = mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0);
    return bias_matrix;
}
