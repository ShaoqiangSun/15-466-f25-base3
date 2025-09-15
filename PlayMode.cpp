#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});


Load< Sound::Sample > honk_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("honk.wav"));
});

Load< Sound::Sample > guitar_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("guitar.wav"));
});

Load< Sound::Sample > violin_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("violin.wav"));
});

Load< Sound::Sample > cello_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("cello.wav"));
});

Load< Sound::Sample > piano_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("piano.wav"));
});

Load< Sound::Sample > piple_organ_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("pipe organ.wav"));
});

Load< Sound::Sample > flute_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("flute.wav"));
});



PlayMode::PlayMode() : scene(*hexapod_scene) {

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	// leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
	// leg_tip_loop = Sound::loop_3D(*guitar_sample, 1.0f, get_leg_tip_position(), 10.0f);
	samples = {guitar_sample, violin_sample, cello_sample, piano_sample, piple_organ_sample, flute_sample};

	sequence = {0, 1, 2, 3, 4, 5};
	std::random_device ran_dev;
	std::mt19937 g(ran_dev());

	std::shuffle(sequence.begin(),sequence.end(), g);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_ESCAPE) {
			SDL_SetWindowRelativeMouseMode(Mode::window, false);
			return true;
		} else if (evt.key.key == SDLK_A) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} 
		// else if (evt.key.key == SDLK_SPACE) {
		// 	if (honk_oneshot) honk_oneshot->stop();
		// 	honk_oneshot = Sound::play_3D(*honk_sample, 0.3f, glm::vec3(4.6f, -7.8f, 6.9f)); //hardcoded position of front of car, from blender
		// }
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		if (SDL_GetWindowRelativeMouseMode(Mode::window) == false) {
			SDL_SetWindowRelativeMouseMode(Mode::window, true);
			return true;
		}
	} 
	
	if ((game_win || game_lose) && evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_R) {
			Mode::set_current(std::make_shared<PlayMode>());
			return true;
		}
	}

	if (game_win || game_lose) return false;

	if (game_state == GameState::WaitingPlayerInput && evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_1) {
			player_input = 1;
		}
		else if (evt.key.key == SDLK_2) {
			player_input = 2;
		}
		else if (evt.key.key == SDLK_3) {
			player_input = 3;
		}
		else if (evt.key.key == SDLK_4) {
			player_input = 4;
		}
		else if (evt.key.key == SDLK_5) {
			player_input = 5;
		}
		else if (evt.key.key == SDLK_6) {
			player_input = 6;
		}
		
		if (player_input - 1 == sample_index) {
			current_sample->stop();
			game_state = GameState::Playing;
			timer = 0.0f;
			player_input = -1;

			if (seq_index == sequence.size()) {
				game_win = true;		
			}
		}
		else {
			incorrect_count++;
			current_sample->stop();

			if (incorrect_count == max_incorrect_count) {
				game_lose = true;
				
				return false;
			}

			current_sample = Sound::play(*samples[sample_index], 1.0f, 0.0f);
			float duration = float(samples[sample_index]->data.size()) / 48000.0f;
			timer = duration;
			
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	if (game_win || game_lose) return;

	//move sound to follow leg tip position:
	// leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		// glm::mat4x3 frame = camera->transform->make_parent_from_local();
		// glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		// glm::vec3 frame_forward = -frame[2];

		// camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_parent_from_local();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	timer -= elapsed;
	if (game_state == GameState::Playing) {
		if (timer <= 0.0f && sequence.size() > 0) {
			sample_index = sequence[seq_index];	
			current_sample = Sound::play(*(samples[sample_index]), 1.0f, 0.0f);

			float duration = float(samples[sample_index]->data.size()) / 48000.0f;
			timer = duration;

			seq_index++;
			game_state = GameState::WaitingPlayerInput;
		}
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	// glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));
	glUseProgram(0);

	// glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		float ofs = 2.0f / drawable_size.y;

		std::string prompt = "Remaining chances to be wrong: " + std::to_string(max_incorrect_count - 1 - incorrect_count);
		std::vector<std::string> instruments = {"Guitar", "Violin", "Cello", "Piano", "Pipe Organ", "Flute"};

		if (!game_lose) {
			lines.draw_text(prompt,
				glm::vec3(-aspect + 0.1f * H, -1.0 + 0.5f * H, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			
			lines.draw_text(prompt,
				glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.5f * H + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			
			for (int i = 0; i < instruments.size(); i++) {
				lines.draw_text(instruments[i] + ": Press " + std::to_string(i + 1),
					glm::vec3(-aspect + 0.1f * H, -1.0 + 0.5f * H + (i + 1) * 2 * H, 0.0),
					glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0x00, 0x00, 0x00, 0x00));
				lines.draw_text(instruments[i] + ": Press " + std::to_string(i + 1),
					glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.5f * H + (i + 1) * 2 * H + ofs, 0.0),
					glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			}

			
		}

		if (game_win) {
			constexpr float H1 = 0.7f;
			float W = 0.5f * H1 * 6;
			lines.draw_text("You Win!",
			glm::vec3(-0.5f * W, -0.3f * H1, 0.0),
			glm::vec3(H1, 0.0f, 0.0f), glm::vec3(0.0f, H1, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			lines.draw_text("You Win!",
			glm::vec3(-0.5f * W + ofs, -0.3f * H1 + ofs, 0.0),
			glm::vec3(H1, 0.0f, 0.0f), glm::vec3(0.0f, H1, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		}

		if (game_lose) {
			constexpr float H1 = 0.7f;
			float W = 0.5f * H1 * 7.5f;
			lines.draw_text("You Lose!",
			glm::vec3(-0.5f * W, -0.3f * H1, 0.0),
			glm::vec3(H1, 0.0f, 0.0f), glm::vec3(0.0f, H1, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			lines.draw_text("You Lose!",
			glm::vec3(-0.5f * W + ofs, -0.3f * H1 + ofs, 0.0),
			glm::vec3(H1, 0.0f, 0.0f), glm::vec3(0.0f, H1, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			
		}
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_world_from_local() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
