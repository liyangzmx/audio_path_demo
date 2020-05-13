#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#define LOG_TAG "[[[audio_path_test]]]"

#include <utils/Log.h>
#include <utils/Thread.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/String8.h>
#include <unistd.h>
#include <media/AudioSystem.h>
#include <system/audio.h>

using namespace android;
#define INFO(...) do { fprintf(stdout, __VA_ARGS__); ALOGI(__VA_ARGS__); } while(0)
#define VERB(...) if(g_debug) { do { fprintf(stdout, __VA_ARGS__); ALOGI(__VA_ARGS__); } while(0); }
#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); ALOGE(__VA_ARGS__); } while(0)

static void portToConfig(struct audio_port_config *config, struct audio_port *port) {
    config->id = port->id;
    config->role = port->role;
    config->type = port->type;
    config->sample_rate = port->active_config.sample_rate;
    config->channel_mask = port->active_config.channel_mask;
    config->format = port->active_config.format;
    config->gain.index = port->active_config.gain.index;
    config->config_mask = port->active_config.config_mask;
}

static audio_patch_handle_t handle = (audio_patch_handle_t)0;

int main(int argc, const char *argv[]) {    
    struct audio_patch nPatch = { .id = handle };
    struct audio_port *sourcePort;
    struct audio_port *sinkPort;
    struct audio_port_config *sourceConfig = &nPatch.sources[0];
    struct audio_port_config *sinkConfig = &nPatch.sinks[0];

    INFO("%s: %d\n", __func__, __LINE__);

#define MAX_PORT_GENERATION_SYNC_ATTEMPTS 5
    int attempts = MAX_PORT_GENERATION_SYNC_ATTEMPTS;
    unsigned int generation1;
    unsigned int generation;
    unsigned int numPorts;
    status_t status;
    struct audio_port *nPorts = NULL;

    if (argc > 1) {
        AudioSystem::releaseAudioPatch((audio_patch_handle_t)atoi(argv[1]));
    }

    do {
        if (attempts-- < 0) { status = TIMED_OUT; break; }
        numPorts = 0;
        status = AudioSystem::listAudioPorts(AUDIO_PORT_ROLE_NONE, AUDIO_PORT_TYPE_DEVICE, &numPorts, NULL, &generation1);
        if (status != NO_ERROR) break;
        if (numPorts == 0) { INFO("No port...\n"); return -1; }
        nPorts = (struct audio_port *)realloc(nPorts, numPorts * sizeof(struct audio_port));
        status = AudioSystem::listAudioPorts(AUDIO_PORT_ROLE_NONE, AUDIO_PORT_TYPE_DEVICE, &numPorts, nPorts, &generation);
    } while (generation1 != generation && status == NO_ERROR);

    for(int i = 0; i < numPorts; i++ ) {
        struct audio_port *port = &nPorts[i];
        if(port->type == AUDIO_PORT_TYPE_DEVICE) {
            if(port->ext.device.type == AUDIO_DEVICE_IN_BUILTIN_MIC) {
                // INFO("source: %d\n", (int)port->id);
                sourcePort = port;
            } else if (port->ext.device.type == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
                // INFO("sink: %d\n", (int)port->id);
                sinkPort = port;
            }
        }
    }

    portToConfig(sourceConfig, sourcePort);
    nPatch.num_sources++;
    portToConfig(sinkConfig, sinkPort);
    sinkConfig->sample_rate = sourceConfig->sample_rate;
    sinkConfig->channel_mask = sourceConfig->channel_mask;
    // sinkConfig->gain.index = sourceConfig->gain.index;
    // sinkConfig->format = sinkConfig->format;
    nPatch.num_sinks++;

    status = AudioSystem::createAudioPatch(&nPatch, &handle);

    struct sigaction g_sigact = {
        .sa_sigaction = [](int signum, siginfo_t *, void *) {
            AudioSystem::releaseAudioPatch(handle);
        },
    };
    sigaction(SIGHUP, &g_sigact, nullptr);
    sigaction(SIGINT, &g_sigact, nullptr);

    while(1){ 
        sleep(1);
    }

    return EXIT_SUCCESS;
}