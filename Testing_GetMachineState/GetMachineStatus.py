NO_OF_MACHINES = 4
n_for_rms_status_assignment = 50
state_change_criteria = (0.9 * n_for_rms_status_assignment)
off_current_threshold = 0.5
rms_status_assignment_sample_count = 0
MACHINE_STATES = {0: 'MACHINE_OFF', 1: 'MACHINE_IDLE', 2: 'MACHINE_ON', 3: 'SENSOR_NOT_CONNECTED'}


class GetMachineStatus:
    OFF_count = 0
    IDLE_count = 0
    ON_count = 0

    def __init__(self):
        self.RMS_long_buffer[NO_OF_MACHINES * n_for_rms_status_assignment] = {}
        pass

    def getmachinestatus(self, machine_number, idle_threshold):
        for j in range(n_for_rms_status_assignment):
            if self.RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] >= 0 and self.RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] <= off_current_threshold:
                self.OFF_count += 1
            elif self.RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] >= off_current_threshold and self.RMS_long_buffer[machine_number * n_for_rms_status_assignment + j] <= idle_threshold:
                self.IDLE_count += 1
            else:
                self.ON_count += 1
        if self.OFF_count >= state_change_criteria:
            return 0
        elif self.IDLE_count >= state_change_criteria:
            return 1
        elif self.ON_count >= state_change_criteria:
            return 2
        else:
            if self.OFF_count > self.ON_count and self.IDLE_count > self.ON_count:
                return 0
            elif self.ON_count > self.OFF_count and self.IDLE_count > self.OFF_count:
                return 2
            return -1  # this indicates that we don't need to change our state





#updated_status = getmachinestatus(machine_number, idle_threshold)
