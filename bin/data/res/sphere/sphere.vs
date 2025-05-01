void main()
{
    object_texture_uvs = in_texture_uvs;
    object_light_space_position = light_space_matrix * model_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0);
    object_normal = (rotation_matrix * vec4(in_normal, 1.0)).xyz;
    object_transformed_position = model_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0);
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0);
}