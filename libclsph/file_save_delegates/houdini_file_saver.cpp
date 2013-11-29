#include "houdini_file_saver.h"

#include "../../util/houdini_geo/HoudiniFileDumpHelper.h"

#define OUTPUT_FILE_NAME "frames/frame"

int houdini_file_saver::writeFrameToFile(particle* particles, const simulation_parameters& parameters) {

    //TODO Find a way to use the same struct for the simulation and writing to file
    houdini_Particle *houdini_Particles = new houdini_Particle[parameters.particles_count];

    std::stringstream ss;
    ss << frames_folder_prefix << OUTPUT_FILE_NAME << ++frame_count << ".geo";
    std::string fileName = ss.str();

    for(int i=0;i<parameters.particles_count;i++){
        houdini_Particles[i].px = particles[i].position.s[0];
        houdini_Particles[i].py = particles[i].position.s[1];
        houdini_Particles[i].pz = particles[i].position.s[2];
        houdini_Particles[i].vx = particles[i].velocity.s[0];
        houdini_Particles[i].vy = particles[i].velocity.s[1];
        houdini_Particles[i].vz = particles[i].velocity.s[2];
        //TODO Use actual colors
        houdini_Particles[i].colorR = 
            particles[i].density > 1000.f && particles[i].density <= 2000.f ?
                (particles[i].density - 1000.f) / 1000.f :
                0.f;
        houdini_Particles[i].colorG = 
            particles[i].density >= 0.f && particles[i].density < 1000.f ?
                1.f - (particles[i].density) / 1000.f :
                0.f;
        houdini_Particles[i].colorB = 
            particles[i].density >= 500.f && particles[i].density <= 1000.f ?
                (particles[i].density - 500.f) / 500.f :
                    particles[i].density >= 1000.f && particles[i].density <= 1500.f ? 
                    1.f - (particles[i].density - 1000.f) / 500.f:
                    0.f;
        houdini_Particles[i].mass = parameters.particle_mass;
    }

    std::ofstream fichier(fileName, std::ios::out | std::ios::trunc);
    if (fichier.is_open())
    {
        ConcreteDataProvider dataProvider(houdini_Particles, parameters.particles_count);
        HoudiniFileDumpParticles partsDumper(&dataProvider);
        partsDumper.dump(fichier);

        fichier.close();
    }
    else
    {
        std::cerr << "Error while writing to " << fileName << std::endl;
    }

    delete[] houdini_Particles;

    return 0;
}