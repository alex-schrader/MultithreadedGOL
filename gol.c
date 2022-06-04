/*
 * Swarthmore College, CS 31
 * Copyright (c) 2022 Swarthmore College Computer Science Department,
 * Swarthmore PA
 */

/*
 * This program is a simulation of Game of Life. It's written in C.
 * Written by Alex and Jonathan
 */

/*
 * To run:
 * ./gol file1.txt  0  # run with config file file1.txt, do not print board
 * ./gol file1.txt  1  # run with config file file1.txt, ascii animation
 * ./gol file1.txt  2  # run with config file file1.txt, ParaVis animation
 *
 */
#include "pthreadGridVisi.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "colors.h"

/****************** Definitions **********************/
/* Three possible modes in which the GOL simulation can run */
#define OUTPUT_NONE 0  // with no animation
#define OUTPUT_ASCII 1 // with ascii animation
#define OUTPUT_VISI 2  // with ParaVis animation

/* Used to slow down animation run modes: usleep(SLEEP_USECS);
 * Change this value to make the animation run faster or slower
 */
//#define SLEEP_USECS  1000000
#define SLEEP_USECS 4000000

/* A global variable to keep track of the number of live cells in the
 * world (this is the ONLY global variable you may use in your program)
 */
static int total_live = 0;
static pthread_barrier_t my_barrier;

/* This struct represents all the data you need to keep track of your GOL
 * simulation.  Rather than passing individual arguments into each function,
 * we'll pass in everything in just one of these structs.
 * this is passed to play_gol, the main gol playing loop
 *
 * NOTE: You will need to use the provided fields here, but you'll also
 *       need to add additional fields. (note the nice field comments!)
 * NOTE: DO NOT CHANGE THE NAME OF THIS STRUCT!!!!
 */
struct gol_data
{

  // NOTE: DO NOT CHANGE the names of these 4 fields (but USE them)
  int rows;        // the row dimension
  int cols;        // the column dimension
  int iters;       // number of iterations to run the gol simulation
  int output_mode; // set to:  OUTPUT_NONE, OUTPUT_ASCII, or OUTPUT_VISI

  int numcord; // number of coordinates at the start
  int *grid;   // The 2d array for gol game
  int *grid_2;

  int num_threads;
  int partition;
  int print_partition;
  int *livecell_array;

  /* fields used by ParaVis library (when run in OUTPUT_VISI mode). */
  // NOTE: DO NOT CHANGE their definitions BUT USE these fields
  visi_handle handle;
  color3 *image_buff;
};

struct partition_args
{
  int tid;
  int startRow;
  int endRow;
  int startCol;
  int endCol;
  int totalRow;
  int totalCol;
  struct gol_data data;
};

int *temp;

void *play_gol(void *args1);
void update_visi(struct partition_args *args);
void update_board(struct partition_args *data);
void swap_board(struct partition_args *data);
int check_neighbour(struct gol_data *data, int x, int y);
void *partition(struct partition_args *args);

/* init gol data from the input file and run mode cmdline args */
int init_game_data_from_args(struct gol_data *data, char *argv[]);

// A mostly implemented function, but a bit more for you to add.
/* print board to the terminal (for OUTPUT_ASCII mode) */
void print_board(struct gol_data *data, int round);

/************ Definitions for using ParVisi library ***********/

/* initialization for the ParaVisi library (DO NOT MODIFY) */
int setup_animation(struct gol_data *data);
/* register animation with ParaVisi library (DO NOT MODIFY) */
// int connect_animation(void (*applfunc)(void *args),
// struct gol_data* data);
/* name for visi (you may change the string value if you'd like) */
static char visi_name[] = "GOL!";

/**********************************************************/
int main(int argc, char *argv[])
{
  int ret;
  // struct partition_args args;
  double secs;
  struct timeval start_time, stop_time;
  struct gol_data data;
  pthread_t *thread_array;
  struct partition_args *thread_args;
  /* check number of command line arguments */
  if (argc != 6)
  {
    printf("usage: ./gol infile.txt output_mode[0,1,2] num_threads partition[0,1] print_partition[0,1]");
    printf("(0: no visualization, 1: ASCII, 2: ParaVisi)\n");
    exit(1);
  }
  /* Initialize game state (all fields in data) from information
   * read from input file */
  ret = init_game_data_from_args(&data, argv);
  if(data.num_threads < 1){
    printf("Invalid. Need at least one thread");
    exit(1);
  }
  if (ret != 0)
  {
    printf("Initialization error: file %s, mode %s\n", argv[1], argv[2]);
    exit(1);
  }
  if (pthread_barrier_init(&my_barrier, 0, data.num_threads))
  {
    printf("pthread_barrier_init error\n");
    exit(1);
  }
  ret = gettimeofday(&start_time, NULL);
  thread_array = malloc(data.num_threads * sizeof(pthread_t));
  thread_args = malloc(data.num_threads * sizeof(struct partition_args));
  if (data.output_mode == OUTPUT_VISI)
  {
    setup_animation(&data);
  }

  for (int i = 0; i < data.num_threads; i++)
  {
    thread_args[i].tid = i;
    thread_args[i].data = data;
    pthread_create(&thread_array[i], NULL, play_gol, &thread_args[i]);
  }
  if (data.output_mode == OUTPUT_VISI){
    run_animation(data.handle, data.iters);

  }
  for (int i = 0; i < data.num_threads; i++)
  {
    pthread_join(thread_array[i], NULL);
  }

  /* initialize ParaVisi animation (if applicable) */
  /* ASCII output: clear screen & print the initial board */

  if (data.output_mode == OUTPUT_ASCII)
  {
    if (system("clear"))
    {
      perror("clear");
      exit(1);
    }
    print_board(&data, 0);
  }

  /* Invoke play_gol in different ways based on the run mode */

  if (ret != 0)
  {
    printf("Read start time error");
    exit(1);
  }
  if (data.output_mode == OUTPUT_NONE)
  { // run with no animation
    int i, j;

    /* Print the round number. */
    int live_cell = 0;

    for (i = 0; i < data.rows; ++i)
    {
      for (j = 0; j < data.cols; ++j)
      {
        if (data.grid[i * data.cols + j] == 1)
        {
          live_cell++;
        }
      }
    }
    total_live = live_cell;
    /* Print the total number of live cells. */
    // fprintf(stderr, "Live cells: %d\n\n", live_cell);
  }
  else if (data.output_mode == OUTPUT_ASCII)
  { // run with ascii animation
    if (system("clear"))
    {
      perror("clear");
      exit(1);
    }

    // NOTE: DO NOT modify this call to print_board at the end
    print_board(&data, data.iters);
  }
  ret = gettimeofday(&stop_time, NULL);
  if (ret != 0)
  {
    printf("Read start time error");
    exit(1);
  }

  if (data.print_partition)
  {
    for(int count = 0; count < data.num_threads; count++){
      printf("\ntid  %d: rows:  %d:%d (%d) cols:  %d:%d (%d)\n\n", 
      thread_args[count].tid, thread_args[count].startRow, thread_args[count].endRow, 
      thread_args[count].totalRow, thread_args[count].startCol, thread_args[count].endCol, 
      thread_args[count].totalCol);
    }
  }
  if (data.output_mode != OUTPUT_VISI)
  {
    secs = (stop_time.tv_sec - start_time.tv_sec) * 1000000 + stop_time.tv_usec - start_time.tv_usec;
    secs /= 1000000;

    /* Print the total runtime, in seconds. */
    fprintf(stdout, "Total time: %0.3f seconds\n", secs);
    fprintf(stdout, "Number of live cells after %d rounds: %d\n\n",
            data.iters, total_live);
  }

  free(data.grid);
  free(data.grid_2);
  free(thread_array);
  free(thread_args);
  free(data.livecell_array);
  return 0;
}

/**********************************************************/


/* parition the board in parallel
 * parameters: a pointer to a partition_arg struct
 * returns: none
 * side effects: stores informaiton in args: 
  thread id, starting row and col index, ending row and col index
  total numbers of rows/cols in each thread.
 */
void *partition(struct partition_args *args)
{
  struct gol_data *data = &args->data;
  if (data->partition)
  {
    int numCol = data->cols / data->num_threads;
    int remainder = data->cols - (numCol * data->num_threads);
    if (args->tid < remainder)
    {
      args->startCol = args->tid * (numCol + 1);
      args->endCol = args->startCol + numCol;
    }
    else
    {
      args->startCol = (remainder * (numCol + 1)) + (args->tid - remainder) * (numCol);
      args->endCol = args->startCol + numCol - 1;
    }
    args->startRow = 0;
    args->endRow = data->rows - 1;
  }
  else
  {
    int numRow = data->rows / data->num_threads;
    int remainder = data->rows - (numRow * data->num_threads);
    if (args->tid < remainder)
    {
      args->startRow = args->tid * (numRow + 1);
      args->endRow = args->startRow + numRow;
    }
    else
    {
      args->startRow = (remainder * (numRow + 1)) + (args->tid - remainder) * (numRow);
      args->endRow = args->startRow + numRow - 1;
    }
    args->startCol = 0;
    args->endCol = data->cols - 1;
  }
  args->totalRow = args->endRow - args->startRow + 1;
  args->totalCol = args->endCol - args->startCol + 1;

  return NULL;
}

/* initialize the gol game state from command line arguments
 *       argv[1]: name of file to read game config state from
 *       argv[2]: run mode value
         argv[3]: number of threads
         argv[4]: partition by row or col (0 - row, 1 - col)
         argv[5]: whether to print parition info (0 no, 1 yes)
 * data: pointer to gol_data struct to initialize
 * argv: command line args
 *       argv[1]: name of file to read game config state from
 *       argv[2]: run mode
         argv[3]: number of threads
         argv[4]: partition by row or col (0 - row, 1 - col)
         argv[5]: whether to print parition info (0 no, 1 yes)
 * returns: 0 on success, 1 on error
 */
int init_game_data_from_args(struct gol_data *data, char *argv[])
{
  data->output_mode = atoi(argv[2]);
  data->num_threads = atoi(argv[3]);
  data->partition = atoi(argv[4]);
  data->print_partition = atoi(argv[5]);
  FILE *infile;
  infile = fopen(argv[1], "r");
  if (infile == NULL)
  {
    printf("Invalid file name");
    exit(1);
  }
  int ret = fscanf(infile, "%d%d%d%d", &data->rows, &data->cols, &data->iters, &data->numcord);
  if (ret != 4)
  {
    printf("Incorrect number of parameters read");
    exit(1);
  }
  int *coord;
  coord = malloc(sizeof(int) * data->numcord * 2);
  for (int i = 0; i < 2 * data->numcord; i += 2)
  {
    ret = fscanf(infile, "%d%d", &coord[i], &coord[i + 1]);
    if (ret != 2)
    {
      printf("Incorrect number of initial coordinates read");
      exit(1);
    }
  }
  data->grid = malloc(sizeof(int) * data->rows * data->cols);
  data->grid_2 = malloc(sizeof(int) * data->rows * data->cols);
  for (int i = 0; i < data->rows; i++)
  {
    for (int j = 0; j < data->cols; j++)
    {
      data->grid[i * data->cols + j] = 0;
      data->grid_2[i * data->cols + j] = 0;
    }
  }

  for (int i = 0; i < 2 * data->numcord; i += 2)
  {
    int x = coord[i];
    int y = coord[i + 1];
    data->grid[x * data->cols + y] = 1;
  }
  data->livecell_array = malloc(sizeof(int) * data->num_threads);

  free(coord);
  coord = NULL;
  fclose(infile);

  // use data in argv to edit struct *data to input rows, columns,
  // etc. from parameters in struct gol_data

  return 0;
}
/**********************************************************/
/* the gol application main loop function:
 *  runs rounds of GOL,free
 *   data: pointer to a struct gol_data  initialized with
 *         all GOL game playing state
 */
void *play_gol(void *args1)
{

  /**********************************************************/

  /* Input: a pointer to a gol_data struct
     Output: None
     Side-effect: Depending on the selected mode, simulate
     and visualize the game.
   */

  //  at the end of each round of GOL, determine if there is an
  //  animation step to take based on the output_mode,
  //   if ascii animation:
  //     (a) call system("clear") to clear previous world state from terminal
  //     (b) call print_board function to print current world state
  //     (c) call usleep(SLEEP_USECS) to slow down the animation
  //   if ParaVis animation:
  //     (a) call your function to update the color3 buffer
  //     (b) call draw_ready(data->handle)
  //     (c) call usleep(SLEEP_USECS) to slow down the animation
  struct partition_args *args = (struct partition_args *)args1;
  struct gol_data *data = &args->data;
  
  partition(args);
  
  for (int i = 0; i < data->iters; i++)
  {

    update_board(args);

    if (data->output_mode == OUTPUT_VISI)
    {
      update_visi(args);
      draw_ready(data->handle);
    }
    pthread_barrier_wait(&my_barrier);

    if (args->tid == 0)
    {
      if (data->output_mode == OUTPUT_ASCII)
      {
        print_board(&args->data, i);

        usleep(SLEEP_USECS);
      }
      if (data->output_mode == OUTPUT_VISI)
      {
        usleep(SLEEP_USECS);
      }
    }
    pthread_barrier_wait(&my_barrier);
  }
  return NULL;
}
void swap_board(struct partition_args *data)
{
  temp = data->data.grid;
  data->data.grid = data->data.grid_2;
  data->data.grid_2 = temp;
}

void update_visi(struct partition_args *args)
{

  /* Input: a pointer to a gol_data struct
     Output: None
     Side-effect: updates the color 3 values for
     the paravisi library.
   */

  int i, j, r, c, index, buff_i;
  color3 *buff;

  buff = args->data.image_buff; // just for readability
  r = args->data.rows;
  c = args->data.cols;

  for (i = args->startRow; i < args->endRow + 1; i++)
  {
    for (j = args->startCol; j < args->endCol+1; j++)
    {
      index = i * c + j;
      // translate row index to y-coordinate value
      // in the image buffer, r,c=0,0 is assumed to be the _lower_ left
      // in the grid, r,c=0,0 is _upper_ left.
      buff_i = (r - (i + 1)) * c + j;

      // update animation buffer
      if (args->data.grid_2[index] == 1)
      {
        buff[buff_i] = c3_white;
      }
      if (args->data.grid_2[index] == 0)
      {
        buff[buff_i] = c3_black;
      }
    }
  }
}

void update_board(struct partition_args *data)
{

  /* Input: a pointer to a gol_data struct
   Output: None
   Side-effect: Update the board after each round
   of iteration.
 */

  int curr_live = 0;
  for (int i = data->startRow; i < data->endRow + 1; i++)
  {
    for (int j = data->startCol; j < data->endCol + 1; j++)
    {
      int numNeighbours = check_neighbour(&data->data, i, j);
      // curr_live += 1;

      if (data->data.grid[i * data->data.cols + j] == 1)
      {

        if (numNeighbours <= 1 || numNeighbours >= 4)
        {
          data->data.grid_2[data->data.cols * i + j] = 0;
        }
        else
        {
          data->data.grid_2[data->data.cols * i + j] = 1;
          curr_live += 1;
          // printf("tid %d coordinates: %d, %d\n", data->tid, i, j);
        }
      }
      else
      {
        if (numNeighbours != 3)
        {
          data->data.grid_2[data->data.cols * i + j] = 0;
        }
        else
        {
          data->data.grid_2[data->data.cols * i + j] = 1;
          curr_live += 1;
          // printf("tid %d coordinates: %d, %d\n", data->tid, i, j);
        }
      }
    }
  }
  data->data.livecell_array[data->tid] = curr_live;
  //    if surrounding cells make cell update, update
  //
  swap_board(data);
}

int check_neighbour(struct gol_data *data, int x, int y)
{
  /* Input: a pointer to a gol_data struct, x and y coordinates
  as integers
   Output: number of neighbours of the point being checked.
   Side-effect: None
 */

  // nested 3x3 for loop
  int count = 0;
  for (int i = x - 1; i < x + 2; i++)
  {
    for (int j = y - 1; j < y + 2; j++)
    {
      int row = i;
      int col = j;
      if (i < 0)
      {
        row = data->rows - 1;
      }
      if (j < 0)
      {
        col = data->cols - 1;
      }
      if (i > data->rows - 1)
      {
        row = 0;
      }
      if (j > data->cols - 1)
      {
        col = 0;
      }
      if (data->grid[row * data->cols + col] == 1 && !(row == x && col == y))
      {
        count++;
      }
    }
  }
  return count;
}
/**********************************************************/
/* Print the board to the terminal.
 *   data: gol game specific data
 *   round: the current round number
 */

void print_board(struct gol_data *data, int round)
{
  /* Input: a pointer to a gol_data struct, current round as
  an integer
     Output: None
     Side-effect: prints the board and round number
   */
  int i, j;

  /* Print the round number. */
  int live_cell = 0;
  system("clear");
  fprintf(stderr, "Round: %d\n", round);

  for (i = 0; i < data->rows; ++i)
  {
    for (j = 0; j < data->cols; ++j)
    {
      if (data->grid[i * data->cols + j] == 1)
      {
        fprintf(stderr, " @");
      }
      else
      {
        fprintf(stderr, " .");
      }
    }
    fprintf(stderr, "\n");
  }

  for (int i = 0; i < data->num_threads; i++)
  {
    // printf("%d ",data->livecell_array[i]);
    live_cell += data->livecell_array[i];
  }
  printf("\n");
  total_live = live_cell;
  /* Print the total number of live cells. */
  fprintf(stderr, "Live cells: %d\n\n", total_live);
}

/**********************************************************/
/***** START: DO NOT MODIFY THIS CODE *****/
/* initialize ParaVisi animation */
int setup_animation(struct gol_data *data)
{
  /* connect handle to the animation */
  int num_threads = data->num_threads;
  data->handle = init_pthread_animation(num_threads, data->rows,
                                        data->cols, visi_name);
  if (data->handle == NULL)
  {
    printf("ERROR init_pthread_animation\n");
    exit(1);
  }
  // get the animation buffer
  data->image_buff = get_animation_buffer(data->handle);
  if (data->image_buff == NULL)
  {
    printf("ERROR get_animation_buffer returned NULL\n");
    exit(1);
  }
  return 0;
}

/* sequential wrapper functions around ParaVis library functions */
void (*mainloop)(struct gol_data *data);

void *seq_do_something(void *args)
{
  mainloop((struct gol_data *)args);
  return 0;
}
/*
Function below is commented out an replaced by a loop that create threads in 
main.

int connect_animation(void (*applfunc)(struct gol_data *data),
    struct gol_data* data)
{
  pthread_t pid;

  mainloop = applfunc;
  if( pthread_create(&pid, NULL, seq_do_something, (void *)data) ) {
    printf("pthread_created failed\n");
    return 1;
  }
  return 0;
}
*/
/***** END: DO NOT MODIFY THIS CODE *****/
/******************************************************/
