#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pulse/pulseaudio.h>

static pa_mainloop *mainloop;
static pa_context *context;
static pa_sink_input_info *sink_input_info = NULL;

static void volume_success(pa_context *c, int success, void *userdata)
{
	printf("Volume success: %i\n", success);
}

static void set_volume(pa_sink_input_info *info, pa_volume_t volume) {
	pa_cvolume cv;
	pa_cvolume_set(&cv, 2, volume);  // Установить громкость для одного канала
	printf("Sink Input Index: %u\n", info->index);
	printf("  Name: %s\n", info->name);
	printf("  Volume: %u\n", info->volume.values[0]);
	pa_context_set_sink_input_volume(context, info->index, &cv, volume_success, NULL);
}

void noshout(pa_sink_input_info *info) {
	const pa_volume_t target_volume = PA_VOLUME_NORM / 1.5;
	static pa_volume_t current_volume = 0;
	static int increasing = 1;

	// Плавное изменение громкости
	if (increasing) {
		if (current_volume < target_volume) {
			current_volume += PA_VOLUME_NORM / 100; // Увеличиваем громкость
		} else {
			increasing = 0; // Достигли максимума, начинаем уменьшать
		}
	} else {
		if (current_volume > 0) {
			current_volume -= PA_VOLUME_NORM / 100; // Уменьшаем громкость
		} else {
			increasing = 1; // Достигли минимума, начинаем увеличивать
		}
	}
	set_volume(sink_input_info, current_volume);
}

static void pa_list_sink_input_callback(pa_context *c, const pa_sink_input_info *info, int eol, void *userdata) {
	if (eol) {
		return;
	}
	if (strstr(info->name, "Tel")) {
		printf("Sink Input Index: %u\n", info->index);
		printf("  Name: %s\n", info->name);
		printf("  Volume: %u\n", info->volume.values[0]);
		size_t s = sizeof(*info);
		sink_input_info = malloc(s);
		memcpy(sink_input_info, info, s); // Сохраняем последний найденный sink input
	}
}

static void context_state_callback(pa_context *c, void *userdata) {
	if (pa_context_get_state(c) == PA_CONTEXT_READY) {
		printf("Connected to PulseAudio.\n");
		// Получение списка sink inputs
		pa_context_get_sink_input_info_list(c, (pa_sink_input_info_cb_t)pa_list_sink_input_callback, NULL);
	}
}

static void signal_handler(int signum) {
	if (signum == SIGINT) {
		printf("\nExiting gracefully...\n");
		pa_context_unref(context);
		pa_mainloop_free(mainloop);
		exit(0);
	}
}

int main(int argc, char *argv[]) {
	mainloop = pa_mainloop_new();
	pa_mainloop_api *mainloop_api = pa_mainloop_get_api(mainloop);
	context = pa_context_new(mainloop_api, "Volume Control Example");

	signal(SIGINT, signal_handler);

	pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
	pa_context_set_state_callback(context, context_state_callback, NULL);

	pa_mainloop_iterate(mainloop, 0, NULL);

	// Основной цикл изменения громкости
	while (1) {
		if (sink_input_info) 
			noshout(sink_input_info);
		pa_mainloop_iterate(mainloop, 0, NULL);
		usleep(100000); // Пауза 0.1 секунды
		pa_context_get_sink_input_info_list(context, (pa_sink_input_info_cb_t)pa_list_sink_input_callback, NULL);
		pa_mainloop_iterate(mainloop, 0, NULL);
	}

	// Очистка ресурсов
	pa_context_unref(context);
	pa_mainloop_free(mainloop);

	return 0;
}
