#include "common.cl.h"
#include "smoothing.cl.inl"
#include "forces.cl.h"
#include "collisions.cl.h"
#include "grid.h"
#include "sort.cl"

void kernel step_1(
    global const particle* input_data,
    global particle* output_data,
    const simulation_parameters params,
    global const unsigned int* cell_table) {

    /* we'll get the same amount of global_ids as there are particles */
    size_t current_particle_index = get_global_id(0);

    output_data[current_particle_index] = input_data[current_particle_index];

    float density = compute_density_with_grid(current_particle_index, input_data, params, cell_table);

    output_data[current_particle_index].density = density;

    //Tait equation more suitable to liquids than state equation
    output_data[current_particle_index].pressure =
        params.K * (pown(density / params.fluid_density, 7) - 1.f);
}

void kernel step_2(
    global const particle* input_data,
    global particle* output_data,
    simulation_parameters params,
    collision_volumes volumes, 
    global const unsigned int* cell_table) {

    size_t current_particle_index = get_global_id(0);

    output_data[current_particle_index] = input_data[current_particle_index];

    float3 acceleration =
        compute_internal_forces_with_grid(current_particle_index, input_data, params, cell_table) /
        input_data[current_particle_index].density;

    acceleration += input_data[current_particle_index].constant_acceleration;

    //Leapfrog
    float3 next =
        input_data[current_particle_index].intermediate_velocity +
        acceleration * params.time_delta * params.simulation_scale;

    float3 position =
        input_data[current_particle_index].position +
        next * (params.time_delta * params.simulation_scale);

    collision_response response = handle_collisions(position, next, params, volumes);

    output_data[current_particle_index].velocity =
        (input_data[current_particle_index].intermediate_velocity + response.next_velocity) / 2.f;
    output_data[current_particle_index].intermediate_velocity = response.next_velocity;
    output_data[current_particle_index].position = response.position;

}
