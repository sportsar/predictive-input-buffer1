// Predictive Input Buffer - recreacion basada en el log de la consola (v4.7)
//
// Idea: en lugar de aplicar cada input (press/release) al instante, se encola
// y se aplica N "ticks" fisicos despues. N depende de la etapa activa y de si
// el jugador esta en modo wave. F6 cicla: STAGE 1 -> 2 -> 3 -> 4 -> OFF.
//
// 1 tick = 1/240 s  (4 ticks = 1 frame @ 60fps, igual que en el log original)

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <deque>

using namespace geode::prelude;

namespace pib {
    struct Stage { int all; int wave; };
    constexpr Stage STAGES[4] = { {2, 2}, {4, 4}, {8, 8}, {16, 8} };
    constexpr int OFF = 4; // indice 4 = desactivado

    inline int g_stage = 0; // arranca en STAGE 1

    inline bool verbose() {
        return Mod::get()->getSettingValue<bool>("verbose-log");
    }

    inline void logStage() {
        if (g_stage == OFF) {
            log::info("[InputBuffer] F6 Cycle -> STAGE 1: OFF (100% Raw Input)");
            return;
        }
        auto s = STAGES[g_stage];
        log::info(
            "[InputBuffer] F6 Cycle -> STAGE {}: {} Ticks All | {} Ticks Wave ({:.1f}f @ 60fps)",
            g_stage + 1, s.all, s.wave, s.all / 4.0f
        );
    }
}

struct PendingInput {
    bool down;
    int button;
    bool player1;
    int remaining; // ticks que faltan para aplicarlo
};

class $modify(PIBBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<PendingInput> queue;
        bool applying = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        auto f = m_fields.self();

        // Pasar directo si esta apagado o si estamos reinyectando un input
        if (f->applying || pib::g_stage == pib::OFF) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto* player = isPlayer1 ? m_player1 : m_player2;
        bool isWave = player && player->m_isDart;
        auto stage = pib::STAGES[pib::g_stage];
        int delay = isWave ? stage.wave : stage.all;

        f->queue.push_back({ down, button, isPlayer1, delay });

        if (pib::verbose()) {
            log::info(
                "*** BUFFERED +{} TICKS ({:.2f} frames @ 60fps) {}! [Limit: {}] ***",
                delay, delay / 4.0f, down ? "PRESS" : "RELEASE", delay
            );
        }
    }

    // Se llama una vez por paso de fisica: aqui descontamos ticks.
   void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto f = m_fields.self();

        for (auto it = f->queue.begin(); it != f->queue.end();) {
            it->remaining -= 1;
            if (it->remaining <= 0) {
                f->applying = true;
                GJBaseGameLayer::handleButton(it->down, it->button, it->player1);
                f->applying = false;
                it = f->queue.erase(it);
            } else {
                ++it;
            }
        }

   GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(PIBPlayLayer, PlayLayer) {
    void resetLevel() {
        // Vaciar la cola al reiniciar para no filtrar inputs entre intentos
        static_cast<PIBBaseGameLayer*>(static_cast<GJBaseGameLayer*>(this))
            ->m_fields->queue.clear();
        PlayLayer::resetLevel();
    }
};

class $modify(PIBKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (down && !repeat && key == KEY_F6) {
            pib::g_stage = (pib::g_stage + 1) % 5;
            pib::logStage();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};

$on_mod(Loaded) {
    log::info("[InputBuffer] v4.7 LOADED (Zero-Lag + Up Arrow Fix)");
    log::info("[InputBuffer] Starting Mode: STAGE 1 (2 All / 2 Wave)");
    log::info("[InputBuffer] Press F6 in-level to cycle: 2/2 -> 4/4 -> 8/8 -> 16/8 -> OFF");
}
