uint8 TYPE_UNKNOWN=0
uint8 TYPE_ROBOT=1
uint8 TYPE_BARRIER=2

# Zero means the cell is empty. Registered object IDs start at one.
int32 occupant_id
int32[] trail_robot_ids
uint8 occupant_type
