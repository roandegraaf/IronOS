#include "fixtures.h"
#include "preview_state.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <zlib.h>

void renderFixture(const Fixture &f, Screen screen, bool leftHanded);

static const int kScale = 4;

struct ScreenInfo {
  Screen      screen;
  const char *name;
};

static const ScreenInfo kScreens[] = {
    {IdleDetailed, "idle_detailed"},           {IdleSimplified, "idle_simplified"}, {SolderingDetailed, "soldering_detailed"},
    {SolderingSimplified, "soldering_simplified"}, {SleepDetailed, "sleep_detailed"},   {SleepSimplified, "sleep_simplified"},
    {SettingsMenu, "settings_menu"},           {WarningRunaway, "warning_thermal_runaway"}, {WarningReset, "warning_settings_reset"},
    {WarningUndervoltage, "warning_undervoltage"},
};

static bool pixelOn(int x, int y) { return previewFramebuffer[(y / 8) * kPanelWidth + x] & (1 << (y & 7)); }

static void putChunk(FILE *out, const char *type, const uint8_t *data, uint32_t len) {
  uint8_t header[8] = {(uint8_t)(len >> 24), (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len, (uint8_t)type[0], (uint8_t)type[1], (uint8_t)type[2], (uint8_t)type[3]};
  fwrite(header, 1, 8, out);
  fwrite(data, 1, len, out);
  uLong crc = crc32(0, header + 4, 4);
  crc       = crc32(crc, data, len);
  uint8_t crcBytes[4] = {(uint8_t)(crc >> 24), (uint8_t)(crc >> 16), (uint8_t)(crc >> 8), (uint8_t)crc};
  fwrite(crcBytes, 1, 4, out);
}

static bool writePNG(const char *path, bool rotate180) {
  const int width  = kPanelWidth * kScale;
  const int height = kPanelHeight * kScale;
  const int stride = width + 1;
  static uint8_t raw[(kPanelWidth * kScale + 1) * kPanelHeight * kScale];
  for (int y = 0; y < height; y++) {
    raw[y * stride] = 0;
    for (int x = 0; x < width; x++) {
      int px = x / kScale;
      int py = y / kScale;
      if (rotate180) {
        px = kPanelWidth - 1 - px;
        py = kPanelHeight - 1 - py;
      }
      raw[y * stride + 1 + x] = pixelOn(px, py) ? 0xFF : 0x00;
    }
  }
  static uint8_t compressed[sizeof(raw) + 1024];
  uLongf         compressedLen = sizeof(compressed);
  if (compress2(compressed, &compressedLen, raw, sizeof(raw), Z_BEST_COMPRESSION) != Z_OK) {
    return false;
  }
  FILE *out = fopen(path, "wb");
  if (!out) {
    return false;
  }
  static const uint8_t signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
  fwrite(signature, 1, 8, out);
  uint8_t ihdr[13] = {0, 0, (uint8_t)(width >> 8), (uint8_t)width, 0, 0, (uint8_t)(height >> 8), (uint8_t)height, 8, 0, 0, 0, 0};
  putChunk(out, "IHDR", ihdr, sizeof(ihdr));
  putChunk(out, "IDAT", compressed, compressedLen);
  putChunk(out, "IEND", nullptr, 0);
  return fclose(out) == 0;
}

static int renderOne(const Fixture &f, Screen screen, bool leftHanded, const char *path) {
  renderFixture(f, screen, leftHanded);
  return writePNG(path, leftHanded) ? 0 : 1;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <output-dir> [name-filter]\n", argv[0]);
    return 2;
  }
  const char *outDir   = argv[1];
  const char *filter   = argc > 2 ? argv[2] : nullptr;
  int         written  = 0;
  int         failures = 0;
  for (const Fixture &f : kFixtures) {
    for (const ScreenInfo &s : kScreens) {
      if (!(f.screens & s.screen)) {
        continue;
      }
      for (int leftHanded = 0; leftHanded < 2; leftHanded++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s--%s--%s.png", outDir, s.name, f.name, leftHanded ? "lh" : "rh");
        if (filter && !strstr(path + strlen(outDir), filter)) {
          continue;
        }
        // One process per render: the firmware keeps state in statics (OLED checksum, menu animation, filters)
        pid_t pid = fork();
        if (pid == 0) {
          _exit(renderOne(f, s.screen, leftHanded, path));
        }
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
          written++;
        } else if (WIFSIGNALED(status)) {
          fprintf(stderr, "failed: %s (signal %d)\n", path, WTERMSIG(status));
          failures++;
        } else {
          fprintf(stderr, "failed: %s (exit %d)\n", path, WEXITSTATUS(status));
          failures++;
        }
      }
    }
  }
  printf("wrote %d PNGs to %s\n", written, outDir);
  return failures ? 1 : 0;
}
