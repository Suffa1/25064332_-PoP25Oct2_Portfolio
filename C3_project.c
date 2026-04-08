#include <stdio.h>
#include <string.h>

#define MAX_CARS 100
#define FEE 5000

struct Car
{
    int id;
    char plate[20];
    int entryTime;
    int exitTime;
};

struct Car cars[MAX_CARS];
int car_count = 0;
int total_fee = 0;
int total_cars = 0;

int save_data() {
    FILE *file = fopen("car.txt", "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return -1;
    }

    fprintf(file, "%d|%d|%d\n", car_count, total_fee, total_cars);
    
    for (int i = 0; i < car_count; i++) {
        fprintf(file, "%d|%s|%d|%d\n", 
                cars[i].id, cars[i].plate, 
                cars[i].entryTime, cars[i].exitTime);
    }

    fclose(file);
    printf("Data saved successfully.\n");
    return 0;
}

int load_data() {
    FILE *file = fopen("car.txt", "r");
    if (file == NULL) {
        printf("No save file found. Starting fresh.\n");
        car_count = 0; total_fee = 0; total_cars = 0;
        return 0;
    }

    car_count = 0; total_fee = 0; total_cars = 0;
    
    char line[256];
    
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return 0;
    }
    
    int count, fee, cars_served;
    if (sscanf(line, "%d|%d|%d", &count, &fee, &cars_served) == 3) {
        car_count = count;
        total_fee = fee;
        total_cars = cars_served;
        printf("Loaded pipe header: %d cars, fee %d, served %d\n", car_count, total_fee, total_cars);
    } 
    else if (sscanf(line, "car count: %d, total fee: %d, total cars: %d", 
                    &count, &fee, &cars_served) == 3) {
        car_count = count;
        total_fee = fee;
        total_cars = cars_served;
        printf("Loaded text header: %d cars, fee %d, served %d\n", car_count, total_fee, total_cars);
    }

    int loaded = 0;
    while (fgets(line, sizeof(line), file) != NULL && loaded < MAX_CARS) {
        int id, entry, exit_time;
        char plate[20];
        
        if (sscanf(line, "%d|%19[^|]|%d|%d", &id, plate, &entry, &exit_time) == 4) {
            cars[loaded].id = id;
            strncpy(cars[loaded].plate, plate, 19);
            cars[loaded].plate[19] = '\0';
            cars[loaded].entryTime = entry;
            cars[loaded].exitTime = exit_time;
            loaded++;
        }
        else if (sscanf(line, "car id: %d, plate: %19[^,], entry time: %d, exit time: %d",
                       &id, plate, &entry, &exit_time) == 4) {
            cars[loaded].id = id;
            strncpy(cars[loaded].plate, plate, 19);
            cars[loaded].plate[19] = '\0';
            cars[loaded].entryTime = entry;
            cars[loaded].exitTime = exit_time;
            loaded++;
        }
    }
    
    car_count = loaded;
    fclose(file);
    
    printf("Successfully loaded %d cars\n", car_count);
    return 0;
}

int save_backup() {
    FILE *src = fopen("car.txt", "r");
    FILE *dst = fopen("car_backup.txt", "w");
    
    if (src == NULL || dst == NULL) {
        printf("Error creating backup.\n");
        if (src) fclose(src);
        if (dst) fclose(dst);
        return -1;
    }
    
    int c;
    while ((c = fgetc(src)) != EOF) {
        fputc(c, dst);
    }
    
    fclose(src);
    fclose(dst);
    printf("Backup saved to car_backup.txt\n");
    return 0;
}

int load_backup() {
    FILE *backup = fopen("car_backup.txt", "r");
    if (backup == NULL) {
        printf("No backup file found.\n");
        return -1;
    }

    FILE *main_save = fopen("car.txt", "w");
    if (main_save == NULL) {
        printf("Error restoring backup.\n");
        fclose(backup);
        return -1;
    }
    
    int c;
    while ((c = fgetc(backup)) != EOF) {
        fputc(c, main_save);
    }
    
    fclose(backup);
    fclose(main_save);
    printf("Backup restored successfully.\n");
    load_data();
    return 0;
}

int reset_data() {
    car_count = 0;
    total_fee = 0;
    total_cars = 0;
    save_data();
    printf("Data reset successfully.\n");
    return 0;
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void showMenu() {
    printf("\n========== PARKING SYSTEM ==========\n");
    printf("1. Car enters parking\n");
    printf("2. Display parked cars\n");
    printf("3. Car leaves parking\n");
    printf("4. Number of parked cars\n");
    printf("5. Statistics\n");
    printf("6. Create backup save\n");
    printf("7. Restore from backup\n");
    printf("8. Reset current save\n");
    printf("0. Exit\n");
}

void addCar() {
    if (car_count >= MAX_CARS) {
        printf("Parking lot is full!\n");
        return;
    }

    struct Car c;
    printf("Enter car ID: ");
    scanf("%d", &c.id);
    clear_input_buffer();

    for(int i = 0; i < car_count; i++) {
        if(cars[i].id == c.id) {
            printf("Error: Car ID %d already exists.\n", c.id);
            return;
        }
    }

    printf("Enter license plate: ");
    scanf("%19s", c.plate);
    clear_input_buffer();
    
    printf("Enter entry time (hour): ");
    scanf("%d", &c.entryTime);
    clear_input_buffer();

    if (c.entryTime < 0 || c.entryTime > 23) {
        printf("Error: Entry time must be between 0 and 23.\n");
        return;
    }
    c.exitTime = -1;
    cars[car_count++] = c;
    printf("Car parked successfully.\n");
    save_data();
}

void showStat() {
    printf("\nPARKING STATISTICS\n");
    printf("Currently parked: %d\n", car_count);
    printf("Total fees collected: %d\n", total_fee);
    printf("Total cars served: %d\n", total_cars);

    int max_duration = -1;
    int max_id = -1;
    char max_plate[20];
    int completed = 0;
    int total_duration = 0;

    for (int i = 0; i < car_count; i++) {
        if (cars[i].exitTime != -1) {
            int duration = cars[i].exitTime - cars[i].entryTime;
            total_duration += duration;
            completed++;
            if (duration > max_duration) {
                max_duration = duration;
                max_id = cars[i].id;
                strncpy(max_plate, cars[i].plate, 19);
                max_plate[19] = '\0';
            }
        }
    }
    
    if (completed > 0) {
        printf("Avg parking duration: %.1f hours\n", (float)total_duration / completed);
        printf("Longest parking: %d hours (ID: %d, %s)\n", max_duration, max_id, max_plate);
    } else {
        printf("No completed parkings yet.\n");
    }
}

void displayCars() {
    if (car_count == 0) {
        printf("No cars currently parked.\n");
        return;
    }

    printf("\nID\tPlate\t\tEntry\tExit\n");
    printf("--------------------------------\n");
    for(int i = 0; i < car_count; i++) {
        printf("%d\t%-15s\t%d\t%d\n",
               cars[i].id, cars[i].plate,
               cars[i].entryTime, cars[i].exitTime);
    }
}

void leaveParking() {
    int id;
    printf("Enter car ID to leave: ");
    scanf("%d", &id);
    clear_input_buffer();

    for(int i = 0; i < car_count; i++) {
        if(cars[i].id == id) {
            printf("Enter exit time (hour 0-23): ");
            scanf("%d", &cars[i].exitTime);
            clear_input_buffer();

            if (cars[i].exitTime < 0 || cars[i].exitTime > 23 || cars[i].exitTime < cars[i].entryTime) {
                printf("Error: Invalid exit time.\n");
                cars[i].exitTime = -1;
                return;
            }

            int duration = cars[i].exitTime - cars[i].entryTime;
            int fee = duration * FEE;
            total_fee += fee;
            total_cars++;
            printf("Duration: %d hours, Fee: %d\n", duration, fee);
            printf("Car %d left parking.\n", id);

            for(int j = i; j < car_count - 1; j++) {
                cars[j] = cars[j+1];
            }
            car_count--;
            save_data();
            return;
        }
    }
    printf("Car ID %d not found.\n", id);
}

int main() {
    load_data();
    int choice;
    while(1) {
        showMenu();
        printf("Choose: ");
        scanf("%d", &choice);
        clear_input_buffer();

        switch(choice) {
            case 1: addCar(); break;
            case 2: displayCars(); break;
            case 3: leaveParking(); break;
            case 4: printf("Parked cars: %d\n", car_count); break;
            case 5: showStat(); break;
            case 6: save_backup(); break;
            case 7: load_backup(); break;
            case 8: reset_data(); break;
            case 0: save_data(); return 0;
            default: printf("Invalid choice.\n");
        }
    }
}