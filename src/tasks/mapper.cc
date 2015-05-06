#include "mapper.hpp"

SolverMapper::SolverMapper
(Machine m, HighLevelRuntime *rt, Processor p)
  : DefaultMapper(m, rt, p) {

  // select and store all SYSTEM_MEM's in valid_mems
  std::set<Memory> all_mems;
  machine.get_all_memories(all_mems);
  std::set<Memory>::const_iterator it_mem = all_mems.begin();
  for (; it_mem != all_mems.end(); it_mem++) {
    if (it_mem->kind() == Memory::SYSTEM_MEM) {
      this->valid_mems.push_back(*it_mem);
      
      // select valid processors for this memory
      std::vector<Processor> valid_procs;
      std::set<Processor> all_procs;
      machine.get_shared_processors(*it_mem, all_procs);
      std::set<Processor>::const_iterator it_proc = all_procs.begin();
      for (; it_proc != all_procs.end(); it_proc++) {
	if ( it_proc->kind() == Processor::LOC_PROC)
	  valid_procs.push_back(*it_proc);
      }

      // map this memory to correspoind processors
      mem_procs[*it_mem] = valid_procs;
    }    
  }
  this->num_mems = valid_mems.size();
  assert( ! valid_mems.empty() );

  // print machine information
  if ( local_proc == mem_procs[valid_mems[0]][0] ) {
    std::cout << "There are " << valid_mems.size()
	      << " machines" << std::endl;
    for (size_t i=0; i<valid_mems.size(); i++) {
      std::cout << "Machine " << i << " has "
		<< mem_procs[ valid_mems[i] ].size()
		<< " cores." << std::endl;
    }
  }
}

void SolverMapper::select_task_options(Task *task) {

  // only top level task is not index space task
  if (! task->is_index_space) {
  
    task->inline_task   = false;
    task->spawn_task    = false;
    task->map_locally   = false;
    task->profile_task  = false;
    task->task_priority = 0;

    // top level task run on machine 0
    std::vector<Processor> procs = mem_procs[ valid_mems[0] ];
    assert( !procs.empty() );  
    task->target_proc = procs[0];
  }

  // index space tasks
  else {

    std::cout << "index space task" << std::endl;  
    task->inline_task   = false;
    task->spawn_task    = false;
    task->map_locally   = true;
    task->profile_task  = false;
    task->task_priority = 0;
  
    // assign a dummy processor
    std::vector<Processor> procs = mem_procs[ valid_mems[0] ];
    assert( !procs.empty() );
    task->target_proc = procs[0];
  }  
}

void SolverMapper::slice_domain(const Task *task, const Domain &domain,
				std::vector<DomainSplit> &slices) {
  
#if 1
  std::cout << "inside slice_domain()" << std::endl;
  std::cout << "orign: " << task->orig_proc.id
	    << ", current: " << task->current_proc.id
	    << ", target: " << task->target_proc.id
	    << std::endl;
#endif

  // pick processors from machine 1
  assert(valid_mems.size()==4);
  Rect<1> r = domain.get_rect<1>();
  int i0 = r.lo[0];
  int i1 = r.lo[0] + r.dim_size(0)/2;
  std::cout << "memory indices: " << i0 << ", "
	    << i1 << std::endl;
  std::vector<Processor> split_set;
  split_set.push_back( mem_procs[ valid_mems[i0] ][0] );
  split_set.push_back( mem_procs[ valid_mems[i1] ][0] );
  
  
  /*
  std::vector<Processor> procs = mem_procs[ valid_mems[1] ];
  std::vector<Processor> split_set;
  for (unsigned idx = 0; idx < 2; idx++) {
    split_set.push_back( procs[0] );
  }
*/
    
  DefaultMapper::decompose_index_space(domain, split_set, 
                                        1, slices);

  for (std::vector<DomainSplit>::iterator it = slices.begin();
        it != slices.end(); it++) {
    
    Rect<1> rect = it->domain.get_rect<1>();
    if (rect.volume() == 1)
      it->recurse = false;
    else
      it->recurse = true;
  }
}

bool SolverMapper::map_task(Task *task) {

#if 1
  std::cout << "Inside map_task() ..." << std::endl;
  std::cout << "orign: " << task->orig_proc.id
	    << ", current: " << task->current_proc.id
	    << ", target: " << task->target_proc.id
	    << std::endl;
#endif

  // Put everything in the system memory
  Memory sys_mem = machine_interface.find_memory_kind
    (task->target_proc, Memory::SYSTEM_MEM); 
  assert(sys_mem.exists());
  for (unsigned idx = 0; idx < task->regions.size(); idx++) {
    task->regions[idx].target_ranking.push_back(sys_mem);
    task->regions[idx].virtual_map = false;
    task->regions[idx].enable_WAR_optimization = war_enabled;
    task->regions[idx].reduction_list = false;
      		
    // make everything SOA
    task->regions[idx].blocking_factor = 1;
    //task->regions[idx].max_blocking_factor;
  } 
  return true;
}

void SolverMapper::notify_mapping_failed(const Mappable *mappable)
{
  printf("WARNING: MAPPING FAILED!  Retrying...\n");
}
/*
void SolverMapper::notify_mapping_result(const Mappable *mappable)
{
  std::cout << "inside notify_result()" << std::endl;
    
#if 0
  if (mappable->get_mappable_kind() == Mappable::TASK_MAPPABLE)
    {
      const Task *task = mappable->as_mappable_task();
      assert(task != NULL);
      for (unsigned idx = 0; idx < task->regions.size(); idx++)
	{
	  printf("Mapped region %d of task %s (ID %lld) to memory %x\n",
		 idx, task->variants->name, 
		 task->get_unique_task_id(),
		 task->regions[idx].selected_memory.id);
	}
    }
#endif
}
*/
