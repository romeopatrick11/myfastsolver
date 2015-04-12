#include "dense_block.hpp"

#include "utility.hpp" // for FIELDID_V
#include <assert.h>

int DenseBlockTask::TASKID;

DenseBlockTask::DenseBlockTask(Domain domain,
			       TaskArgument global_arg,
			       ArgumentMap arg_map,
			       Predicate pred,
			       bool must,
			       MapperID id,
			       MappingTagID tag)
  
  : IndexLauncher(TASKID, domain, global_arg,
		  arg_map, pred, must, id, tag) {}

void DenseBlockTask::register_tasks(void)
{
  TASKID = HighLevelRuntime::register_legion_task
    <DenseBlockTask::cpu_task>(AUTO_GENERATE_ID,
			       Processor::LOC_PROC, 
			       false,
			       true,
			       AUTO_GENERATE_ID,
			       TaskConfigOptions(true/*leaf*/),
			       "Dense_Block");

  //#ifndef SHOW_REGISTER_TASKS
  printf("Register task %d : Dense_Block\n", TASKID);
  //#endif
}

void DenseBlockTask::cpu_task(const Task *task,
			      const std::vector<PhysicalRegion> &regions,
			      Context ctx, HighLevelRuntime *runtime) {

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->arglen == sizeof(TaskArgs));
  assert(task->local_arglen == sizeof(ThreeSeeds));

  Point<1> p = task->index_point.get_point<1>();
  printf("point = %d\n", p[0]);

  const ThreeSeeds seeds = *((const ThreeSeeds*)task->local_args);
  long uSeed = seeds.uSeed;
  long vSeed = seeds.vSeed;
  long dSeed = seeds.dSeed;

  printf("random seeds = (%lu, %lu, %lu) \n", uSeed, vSeed, dSeed);
  
  const TaskArgs matrix = *((const TaskArgs*)task->args);
  int rows = matrix.rows;
  int cols = matrix.cols;
  int rank = matrix.rank;
  int blks = matrix.blocks;
  
  printf("matrix row size = %i\n", rows);
  printf("matrix col size = %i\n", cols);
  printf("rank = %i\n", rank);
  printf("block size = %i\n", blks);
 
  Rect<2> bounds, subrect;
  bounds.lo.x[0] = p[0] * rows;
  bounds.hi.x[0] = (p[0] + 1) * rows - 1;
  bounds.lo.x[1] = 0;
  bounds.hi.x[1] = cols - 1;
  ByteOffset offsets[2];
  double *base = regions[0].get_field_accessor(FIELDID_V).template typeify<double>().template raw_rect_ptr<2>(bounds, subrect, offsets);
  assert(subrect == bounds);
  printf("ptr = %p (%d, %d)\n", base, offsets[0].offset, offsets[1].offset);
  /*
  // recover U
  struct drand48_data buffer;
  assert( srand48_r( uSeed, &buffer ) == 0 );
  for(int ri = 0; ri < rows; ri++)
    for(int ci = 0; ci < cols; ci++) {

      assert( drand48_r(&buffer, value) == 0 );
    }

  double *value = base + ri * offsets[0] + ci * offsets[1];
  */
}


