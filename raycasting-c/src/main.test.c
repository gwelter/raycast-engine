// Unit tests for src/main.c
// Uses CMocka for assertions and test runner

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "constants.h"

// Avoid conflicting entry symbols by renaming the application's main
#define main raycast_app_main
#include "main.c"
#undef main

static int setup_player_default(void **state) {
  (void)state;
  // Initialize a predictable player state for tests
  player.x = SCREEN_WIDTH / 2.0f;
  player.y = SCREEN_HEIGHT / 2.0f;
  player.width = 1.0f;
  player.height = 1.0f;
  player.turnDirection = 0;
  player.walkDirection = 0;
  player.rotationAngle = PI / 2.0f;  // facing down initially
  player.walkSpeed = 150.0f;
  player.turnSpeed = 100.0f * PI / 180.0f;
  return 0;
}

static void test_distance_between_points(void **state) {
  (void)state;
  assert_true(fabs(distanceBetweenPoints(0, 0, 3, 4) - 5.0) < 1e-6);
  assert_true(fabs(distanceBetweenPoints(-1, -1, -4, -5) - 5.0) < 1e-6);
}

static void test_normalize_angle_range(void **state) {
  (void)state;
  float a1 = normalizeAngle(-PI / 2.0f);
  assert_true(a1 >= 0.0f && a1 < TWO_PI);

  float a2 = normalizeAngle(3.0f * PI);
  // 3PI mod 2PI == PI
  assert_true(fabs(a2 - PI) < 1e-6);
}

static void test_hasWallAt_and_wallContentAt(void **state) {
  (void)state;
  // Out of bounds are considered walls
  assert_int_equal(hasWallAt(-1, 10), TRUE);
  assert_int_equal(hasWallAt(SCREEN_WIDTH + 1, 10), TRUE);

  // Inside top-left tile which is a wall
  assert_int_equal(hasWallAt(1, 1), TRUE);

  // Inside an empty tile: map[1][1] == 0
  float x = TILE_SIZE * 1 + 1;
  float y = TILE_SIZE * 1 + 1;
  assert_int_equal(hasWallAt(x, y), FALSE);
  assert_int_equal(wallContentAt(x, y), 0);

  // Known non-zero content: map[2][15] == 8 (see map in main.c)
  float x8 = TILE_SIZE * 15 + 1;
  float y8 = TILE_SIZE * 2 + 1;
  assert_int_equal(wallContentAt(x8, y8), 8);
}

static void test_movePlayer_moves_and_blocks(void **state) {
  (void)state;
  // Starting from center, face right and move forward
  player.rotationAngle = 0.0f;  // face right (increasing x)
  player.walkDirection = 1;
  float startX = player.x;
  float startY = player.y;
  movePlayer(0.5f);  // move for half a second => 75 units
  assert_true(player.x > startX);
  assert_true(fabs(player.y - startY) < 1e-3);

  // Try to move far left into the out-of-bounds wall; should be blocked
  player.rotationAngle = PI;  // face left
  player.walkDirection = 1;
  player.x = TILE_SIZE + 5.0f;
  player.y = TILE_SIZE + 5.0f;
  float bx = player.x;
  float by = player.y;
  movePlayer(10.0f);  // a very large step that would cross the boundary
  assert_true(fabs(player.x - bx) < 1e-6);
  assert_true(fabs(player.y - by) < 1e-6);
}

static void test_castRay_basic(void **state) {
  (void)state;
  // Place player in an open space and cast a ray straight right
  player.x = TILE_SIZE * 10 + 10;
  player.y = TILE_SIZE * 6 + 10;
  float angle = 0.0f;
  castRay(angle, 0);
  assert_true(rays[0].distance > 0.0);
  assert_int_not_equal(rays[0].wallHitContent, 0);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test_setup(test_distance_between_points, setup_player_default),
      cmocka_unit_test_setup(test_normalize_angle_range, setup_player_default),
      cmocka_unit_test_setup(test_hasWallAt_and_wallContentAt, setup_player_default),
      cmocka_unit_test_setup(test_movePlayer_moves_and_blocks, setup_player_default),
      cmocka_unit_test_setup(test_castRay_basic, setup_player_default),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
