//
// Created by pavel on 03.02.23.
//

#ifndef PA1_PIPES_H
#define PA1_PIPES_H

struct pipes {
    int fd[2];
};

typedef struct {
    size_t number_of_child_procs;
    struct pipes ** pipes_all;
} pipes_all_global;

pipes_all_global * new(size_t number) {
    pipes_all_global * glb = (pipes_all_global *) malloc(sizeof(pipes_all_global));
    glb->number_of_child_procs = number;
    glb->pipes_all = (struct pipes **) malloc(sizeof(struct pipes *) * glb->number_of_child_procs);
    for (size_t a = 0; a < glb->number_of_child_procs; ++a) {
        glb->pipes_all[a] = (struct pipes *)  malloc(sizeof(struct pipes) * glb->number_of_child_procs);
    }
    return glb;
}

typedef struct {
    size_t id;
    pipes_all_global * global_elite;
} baby_maybe_process;

baby_maybe_process * new_baby(size_t id, pipes_all_global * x) {
    baby_maybe_process * mb = (baby_maybe_process *) malloc(sizeof(baby_maybe_process));
    mb->id = id;
    mb->global_elite = x;
    return mb;
}

#endif //PA1_PIPES_H
