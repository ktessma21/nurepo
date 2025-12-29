#include "ram.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "object.h"


/**
  * @brief ram_init: initialize memory unit
  *
  * Returns a pointer to a dynamically-allocated memory
  * for storing nuPython variables and their values. All
  * memory cells are initialized to the value None. You
  * take ownership of the returned memory and must call
  * ram_destroy() when you are done.
  *
  * @return pointer to struct denoting memory unit
  */
struct RAM* ram_init(void){
    struct RAM* memory = malloc(sizeof(struct RAM));
    if (!memory)
        return NULL;
    
    memory->size = 0;
    memory->capacity = 16; // initial capacity
    memory->cells = malloc(sizeof(struct RAM_VALUE) * memory->capacity);

    if (!memory->cells){
        free(memory);
        return NULL;
    }

    memory->map = malloc(sizeof(struct RAM_MAP) * memory->capacity);

    if (!memory->map){
        free(memory->cells);
        free(memory);
        return NULL;
    }
    // initialize all cells to NONE
    for (int i = 0; i < memory->capacity; i++){
        memory->cells[i].value_type = RAM_VALUE_NONE;
        memory->cells[i].obj_value  = NULL;  
    }

    return memory;
}

static void ram_clear_cell(struct RAM_VALUE *cell) {
    if (!cell) return;

    if (cell->value_type != RAM_VALUE_NONE && cell->obj_value != NULL) {
        object_free(cell->obj_value);
    }
    cell->value_type = RAM_VALUE_NONE;
    cell->obj_value  = NULL;
}


/**
  * @brief ram_destroy: frees memory associated with memory unit
  * 
  * Frees the dynamically-allocated memory associated with
  * the given memory. After the call returns, you cannot
  * use the memory.
  *
  * @return void
  */
void ram_destroy(struct RAM* memory){
    if (!memory)
        return; 

    if (memory->cells){
        // free each object in cells 
        for (int i = 0; i < memory->capacity; i++ ){
            ram_clear_cell(&memory->cells[i]);
        }
    }
    free(memory->cells);

    if (memory->map){
        for (int i = 0; i < memory->size; i++ ){
            free(memory->map[i].varname);
        }
    }
    free(memory->map);
    free(memory);
}


/**
  * @brief ram_size: # of vars in memory
  *
  * Returns the # of variables currently stored in memory.
  *
  * @return # of vars in memory
  */
int ram_size(struct RAM* memory){
    if (!memory)
        return 0;
    return memory->size;
}


/**
  * @brief ram_capacity: # of cells available in memory
  *
  * Returns the # of cells currently available in memory.
  *
  * @return # of cells available in memory
  */
int ram_capacity(struct RAM* memory){
    if (!memory)
        return 0;
    return memory->capacity;
}


/**
  * @brief ram_get_addr: address of memory cell occupied by variable
  *
  * If the given variable (e.g. "x") has been written to 
  * memory, returns the address of this variable --- an integer
  * in the range 0..N-1 where N is the number of vars currently 
  * stored in memory. Returns -1 if no such variable exists 
  * in memory. 
  *
  * NOTE: a variable has to be written to memory before you can
  * get its address. Once a variable is written to memory, its
  * address never changes. 
  *
  * @param memory Pointer to struct denoting memory unit
  * @param varname variable name
  * @return address of variable or -1 if doesn't exist
  */
int ram_get_addr(struct RAM* memory, char* varname){
    if (!memory || !varname)
        return -1;
    
    int l = 0;
    int r = memory->size - 1;

    while (l <= r){
        int m = l + (r - l) / 2;
        int cmp = strcmp(memory->map[m].varname, varname);
        if (cmp == 0)
            return memory->map[m].cell;
        else if (cmp < 0)
            l = m + 1;
        else 
            r = m - 1;
    }
    return -1;
}

/**
  * @brief ram_read_cell_by_addr: returns value in memory cell at this address
  *
  * Given a memory address (an integer in the range 0..N-1), 
  * returns a COPY of the value contained in that memory cell.
  * Returns NULL if the address is not valid.
  * 
  * NOTE: this function allocates memory for the value that
  * is returned. The caller takes ownership of the copy and 
  * must eventually free this memory via ram_free_value().
  *
  * NOTE: a variable has to be written to memory before its
  * address becomes valid. Once a variable is written to memory,
  * its address never changes.
  *
  * @param memory Pointer to struct denoting memory unit
  * @param address memory cell address
  * @return pointer to struct containing value or NULL if doesn't exist
  */
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address){
    if (!memory || address < 0 || address >= memory->size)
        return NULL;
    
    struct RAM_VALUE* copy = malloc(sizeof(struct RAM_VALUE));
    if (!copy)
        return NULL;

    copy->value_type = memory->cells[address].value_type;

    if (copy->value_type == RAM_VALUE_NONE) {
        // represent None: no object
        copy->obj_value = NULL;
        return copy;
    }

    if (!memory->cells[address].obj_value) {
        // corrupt or uninitialized cell; be defensive
        free(copy);
        return NULL;
    }

    copy->obj_value = malloc(sizeof(struct object));
    if (!copy->obj_value) {
        free(copy);
        return NULL;
    }

    memcpy(copy->obj_value,
           memory->cells[address].obj_value,
           sizeof(struct object));

    return copy;
}

/**
  * @brief ram_read_cell_by_name: returns value in memory cell for this variable
  *
  * If the given variable (e.g. "x") has been written to 
  * memory, returns a COPY of the value contained in memory.
  * Returns NULL if no such name exists in memory.
  *
  * NOTE: this function allocates memory for the value that
  * is returned. The caller takes ownership of the copy and 
  * must eventually free this memory via ram_free_value().
  *
  * @param memory Pointer to struct denoting memory unit
  * @param varname variable name
  * @return pointer to struct containing value or NULL if doesn't exist
  */
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* varname){
    if (!memory || !varname)
        return NULL;
    
    int addr = ram_get_addr(memory, varname);
    if (addr == -1)
        return NULL;

    return ram_read_cell_by_addr(memory, addr);
}

/**
  * @brief ram_free_value: free value returned by read_cell() functions
  *
  * Frees the memory value returned by ram_read_cell_by_name and
  * ram_read_cell_by_addr.
  *
  * @param value Pointer to struct containing value
  * @return void
  */
void ram_free_value(struct RAM_VALUE* value){
    if (!value)
        return;
    
    if (value->obj_value) {
        object_free(value->obj_value);
    }
    free(value);
}


/**
  * @brief ram_write_cell_by_addr: writes a value to memory cell at this address
  *
  * Writes the given value to the memory cell at the given 
  * address. If a value already exists at this address, that
  * value is overwritten by this new value. Returns true if 
  * the value was successfully written, false if not (which 
  * implies the memory address is invalid).
  *
  * NOTE: if the value being written is a string, it will
  * be duplicated and stored.
  * 
  * NOTE: a variable has to be written to memory before its
  * address becomes valid. Once a variable is written to memory,
  * its address never changes.
  *
  * @param memory Pointer to struct denoting memory unit
  * @param value value to be written to memory
  * @param address memory cell address
  * @return true if successful, false if not (invalid address)
  */
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address){
    if (!memory || address < 0 || address >= memory -> size)
        return false;

    ram_clear_cell(&memory->cells[address]);
    memory->cells[address].value_type = RAM_VALUE_NONE;

    // If writing "None", nothing else to do
    if (value.value_type == RAM_VALUE_NONE) {
        memory->cells[address].obj_value = NULL;
        return true;
    }

    memory->cells[address].obj_value = malloc(sizeof(struct object));

    if (!memory->cells[address].obj_value) {
            memory->cells[address].value_type = RAM_VALUE_NONE;
            return false;
        }
    memcpy(memory->cells[address].obj_value,
        value.obj_value,
        sizeof(struct object));

    return true;


    // memccpy(memory->cells[address].obj_value, value)
}
/**
  * ram_write_cell_by_name
  *
  * Writes the given value to a memory cell named by the given
  * variable. If a memory cell already exists with this name,
  * the existing value is overwritten by this new value. Returns
  * true since this operation always succeeds.
  *
  * NOTE: if the value being written is a string, it will
  * be duplicated and stored.
  *
  * NOTE: a variable has to be written to memory before its
  * address becomes valid. Once a variable is written to memory,
  * its address never changes.
  *
  * @param memory Pointer to struct denoting memory unit
  * @param value value to be written to memory
  * @param varname variable name
  * @return true (always successful)
  */
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* varname){
    if (!memory || !varname)
        return false;
    
    int addr = ram_get_addr(memory, varname);
    if (addr != -1)
        return ram_write_cell_by_addr(memory, value, addr);
    
    // need to add new variable 
    // resize if needed 
    if (memory->size >= memory->capacity){
        int new_capacity = memory->capacity*2; 
        struct RAM_VALUE* new_cells = realloc(memory->cells , sizeof(struct RAM_VALUE)*new_capacity);
        if (!new_cells)
            return false;
        
        memory->cells = new_cells; // always realloc with a new pointer and then assign if succesful 
        struct RAM_MAP* new_map = realloc(memory->map, sizeof(struct RAM_MAP)*new_capacity);
        if (!new_map)
            return false;
        memory->map = new_map;
        memory->capacity = new_capacity;
    }
    
    memory->size++;

    // insert into map in correct order 
    int l = 0;
    int r = memory->size - 2;
    while ( l <= r){
        int m = l + (r - l) / 2;
        int cmp = strcmp(memory->map[m].varname, varname);
        if (cmp < 0)
            l = m + 1;
        else
            r = m - 1;
    }
    
    // shift right to make room at index l
    for (int i = memory->size - 1; i > l; i--){
        memory->map[i] = memory->map[i-1];
    }

    // now fill slot l
    memory->map[l].varname = strdup(varname);
    if (!memory->map[l].varname) {
        // rollback size and leave RAM in a consistent state
        for (int i = l; i < memory->size - 1; i++) {
            memory->map[i] = memory->map[i+1];
        }
        memory->size--;
        return false;
    }
    memory->map[l].cell = memory->size - 1;

    return ram_write_cell_by_addr(memory, value, memory->size - 1);
}




/**
 * @brief ram_print: prints the contents of memory
 *
 * Prints the contents of RAM to the console, for debugging.
 * RAM is printed in alphabetical order by variable name.
  *
  * @param memory Pointer to struct denoting memory unit
  * @return void
  */
void ram_print(struct RAM* memory){
    if (!memory)
        return;
    
    printf("RAM contents:\n");
    for (int i = 0; i < memory->size; i++){
        printf("Var: %s, Cell: %d, Type: %d\n",
               memory->map[i].varname,
               memory->map[i].cell,
               memory->cells[memory->map[i].cell].value_type);      
    }

}

/**
  * @brief ram_print_map: prints the contents of memory map
  *
  * Prints contents of RAM map to the console, for debugging.
  *
  * @param memory Pointer to struct denoting memory unit
  * @return void
  */
void ram_print_map(struct RAM* memory){
    if (!memory)
        return;
    
    printf("RAM map contents:\n");
    for (int i = 0; i < memory->size; i++){
        printf("Var: %s, Cell: %d\n",
               memory->map[i].varname,
               memory->map[i].cell);      
    }
}