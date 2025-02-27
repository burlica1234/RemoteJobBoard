#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 12347
#define BUFFER_SIZE 1024
#define USER_DB "useri.xml"
#define JOBS_DB "jobs.xml"
#define CVS_DB "cvs.xml"
#define FAVOURITES_DB "favourites.xml"
#define RSP_DB "responses.xml"
// functii pt initializare bd
void initialize_user_db() {
    FILE *file = fopen(USER_DB, "r");
    if (!file) {
        file = fopen(USER_DB, "w");
        if (file) {
            fprintf(file, "<users>\n</users>\n");
            fclose(file);
        }
    } else 
        fclose(file);
    
}
void initialize_responses_db() {
    FILE *file = fopen(RSP_DB, "r");
    if (!file) {
        file = fopen(RSP_DB, "w");
        if (file) {
            fprintf(file, "<responses>\n</responses>\n");
            fclose(file);
        }
    } else 
        fclose(file);
    
}
void initialize_jobs_db() {
    FILE *file = fopen(JOBS_DB, "r");
    if (!file) {
        file = fopen(JOBS_DB, "w");
        if (file) {
            fprintf(file, "<jobs>\n</jobs>\n");
            fclose(file);
        }
    } else 
        fclose(file);
    
}
void initialize_cvs_db() {
    FILE *file = fopen(CVS_DB, "r");
    if (!file) {
        file = fopen(CVS_DB, "w");
        if (file) {
            fprintf(file, "<cvs>\n</cvs>\n");
            fclose(file);
        }
    } else 
        fclose(file);
    
}
void initialize_favourites_db() {
    FILE *file = fopen(FAVOURITES_DB, "r");
    if (!file) {
        file = fopen(FAVOURITES_DB, "w");
        if (file) {
            fprintf(file, "<favourites>\n</favourites>\n");
            fclose(file);
        }
    } else 
        fclose(file);
}

int validate_user(const char *username, const char *password, char *status) {
    FILE *file = fopen(USER_DB, "r");
    if (!file) return 0;

    char s[BUFFER_SIZE];
    while (fgets(s, sizeof(s), file)) {
        if (strstr(s, "<user") && strstr(s, username) && strstr(s, password)) {
            char *p = strstr(s, "status=\"");  // folosim \ pentru a nu fi interpretat statusul gresit de catre compilator,problema la ghilimele
            if (p) {
                sscanf(p, "status=\"%[^\"]\"", status);
                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);
    return 0;
}
// adaugam joburi favorite in bd
void add_to_favourites(const char *username, const char *job_title) {
    FILE *file = fopen(FAVOURITES_DB, "r+");
    if (!file) return;
    
    fseek(file, -13, SEEK_END);//inainte de a se termina </favourites>
    fprintf(file, "    <favourite>\n");
    fprintf(file, "        <username>%s</username>\n", username);
    fprintf(file, "        <job_title>%s</job_title>\n", job_title);
    fprintf(file, "    </favourite>\n</favourites>\n");
    fclose(file);
}
void get_favourites(const char *username, char *response) {
    FILE *file = fopen(FAVOURITES_DB, "r");
    if (!file) {
        strcpy(response, "No favourites found.");
        return;
    }

    char v[BUFFER_SIZE];
    char fav_username[BUFFER_SIZE] = {0};
    char fav_job_title[BUFFER_SIZE] = {0};
    int ok = 0;

    strcpy(response, ""); // golim pt raspuns
    while (fgets(v, sizeof(v), file)) {
        if (strstr(v, "<username>")) {
            sscanf(v, " <username>%[^<]</username>", fav_username);
            if (strcmp(fav_username, username) == 0)  //comparam pentru a diferentia job urile favorite pt fiecare user
                ok = 1;
            else 
                ok = 0;
            
        } else if (ok && strstr(v, "<job_title>")) {
            sscanf(v, " <job_title>%[^<]</job_title>", fav_job_title);//citeste tot intre job title pana la intalnirea primului <
            strcat(response, fav_job_title);
            strcat(response, "\n"); 
        }
    }

    fclose(file);

    if (strlen(response) == 0) 
        strcpy(response, "No favourites found.");
    
}
int register_user(const char *username, const char *password, const char *status) {
    FILE *file = fopen(USER_DB, "r+");
    if (!file) return 0;

    char s[BUFFER_SIZE];
    while (fgets(s, sizeof(s), file)) {
        if (strstr(s, "<user") && strstr(s, username)) {
            fclose(file);
            return 0; // exista useru deja
        }
    }

    fseek(file, -9, SEEK_END); // inainte de </users>
    fprintf(file, "    <user username=\"%s\" password=\"%s\" status=\"%s\" />\n</users>\n", username, password, status);
    fclose(file);
    return 1;
}
void add_job(const char *title, const char *description, const char *salary) {
    FILE *file = fopen(JOBS_DB, "r+");
    if (!file) return;

    fseek(file, -8, SEEK_END); // incepem sa scriem inainte de </jobs>
    fprintf(file, "    <job>\n");
    fprintf(file, "        <title>%s</title>\n", title);
    fprintf(file, "        <description>%s</description>\n", description);
    fprintf(file, "        <salary>%s</salary>\n", salary);
    fprintf(file, "    </job>\n</jobs>\n");

    fclose(file);
}

// citire joburi din baza de date pt a putea fi afisate
void get_all_jobs(char *response) {
    FILE *file = fopen(JOBS_DB, "r");
    if (!file) {
        strcpy(response, "No jobs available.");
        return;
    }

    char s[BUFFER_SIZE];
    char titlu[BUFFER_SIZE] = {0};
    char descriere[BUFFER_SIZE] = {0};
    char salar[BUFFER_SIZE] = {0};

    strcpy(response, "");

    while (fgets(s, sizeof(s), file)) {
        if (strstr(s, "<title>")) 
            sscanf(s, " <title>%[^<]</title>", titlu);
        else if (strstr(s, "<description>")) 
            sscanf(s, " <description>%[^<]</description>", descriere);
        else if (strstr(s, "<salary>")) {
            sscanf(s, " <salary>%[^<]</salary>", salar);

            char job_details[BUFFER_SIZE];
            snprintf(job_details, BUFFER_SIZE, "Title: %s\nDescription: %s\nSalary: %s\n\n", titlu, descriere, salar);
            strcat(response, job_details);

            memset(titlu, 0, sizeof(titlu));
            memset(descriere, 0, sizeof(descriere));//curatam pentru a pregati functia pt urmatorul job
            memset(salar, 0, sizeof(salar));
        }
    }

    fclose(file);

    if (strlen(response) == 0) 
        strcpy(response, "No jobs available.");
    
}

void add_cv(const char *job_title, const char *name, const char *email, const char *experience) {
    FILE *file = fopen(CVS_DB, "r+");
    if (!file) return;

    fseek(file, -7, SEEK_END); // incepem sa scriem inainte de </cvs>
    fprintf(file, "    <cv>\n");
    fprintf(file, "        <job_title>%s</job_title>\n", job_title);
    fprintf(file, "        <name>%s</name>\n", name);
    fprintf(file, "        <email>%s</email>\n", email);
    fprintf(file, "        <experience>%s</experience>\n", experience);
    fprintf(file, "    </cv>\n</cvs>\n");

    fclose(file);
}
void get_cvs_for_job(const char *job_title, char *response) {
    FILE *file = fopen(CVS_DB, "r");
    if (!file) {
        strcpy(response, "No CVs available.");
        return;
    }

    char n[BUFFER_SIZE];
    char temptitlu[BUFFER_SIZE] = {0};
    char nume[BUFFER_SIZE] = {0}, email[BUFFER_SIZE] = {0}, exp[BUFFER_SIZE] = {0};
    int ok = 0;

    
    char clean_title[BUFFER_SIZE];//variabila curatata pentru a nu intampina probleme cu diferite caractere care nu s la locul lor
    snprintf(clean_title, sizeof(clean_title), "%s", job_title);
    clean_title[BUFFER_SIZE - 1] = '\0';

    strcpy(response, "");

    while (fgets(n, sizeof(n), file)) {
        if (strstr(n, "<job_title>")) {
            sscanf(n, " <job_title>%[^<]</job_title>", temptitlu);
            if (strcmp(temptitlu, clean_title) == 0) 
                ok = 1;
            else 
                ok = 0;
        }    
        else if (ok && strstr(n, "<name>")) 
            sscanf(n, " <name>%[^<]</name>", nume);
        else if (ok && strstr(n, "<email>"))
            sscanf(n, " <email>%[^<]</email>", email);
        else if (ok && strstr(n, "<experience>")) {
            sscanf(n, " <experience>%[^<]</experience>", exp);

            char detalii[BUFFER_SIZE];
            snprintf(detalii, BUFFER_SIZE, "Name: %s\nEmail: %s\nExperience: %s\n\n", nume, email, exp);
            strcat(response, detalii);

            memset(nume, 0, sizeof(nume));
            memset(email, 0, sizeof(email));//eliberam pt next cv
            memset(exp, 0, sizeof(exp));
        }
    }

    fclose(file);

    if (strlen(response) == 0) 
        strcpy(response, "No CVs available for this job.");
    
}
void save_response(const char *job_title, const char *email, const char *message) {
    FILE *file = fopen(RSP_DB, "r+");
    if (!file) return;

    fseek(file, -12, SEEK_END); 
    fprintf(file, "    <response>\n");
    fprintf(file, "        <job_title>%s</job_title>\n", job_title);
    fprintf(file, "        <email>%s</email>\n", email);
    fprintf(file, "        <message>%s</message>\n", message);
    fprintf(file, "    </response>\n</responses>\n");

    fclose(file);
}

void get_responses_for_employee(const char *email, char *response) {
    FILE *file = fopen(RSP_DB, "r");
    if (!file) {
        strcpy(response, "No responses found.");
        return;
    }

    char line[BUFFER_SIZE];
    char temp_job_title[BUFFER_SIZE] = {0};
    char temp_email[BUFFER_SIZE] = {0};
    char temp_message[BUFFER_SIZE] = {0};
    int ok = 0;  

    strcpy(response, "");  

    while (fgets(line, sizeof(line), file)) {
      
        if (strstr(line, "<response>")) {
            // golim toate campurile
            memset(temp_job_title, 0, sizeof(temp_job_title));
            memset(temp_email, 0, sizeof(temp_email));
            memset(temp_message, 0, sizeof(temp_message));
            ok = 0;  
        }

        
        if (strstr(line, "<email>")) {
            sscanf(line, " <email>%[^<]</email>", temp_email);
            if (strcmp(temp_email, email) == 0) 
                ok = 1;  
        }

        if (strstr(line, "<job_title>")) 
            sscanf(line, " <job_title>%[^<]</job_title>", temp_job_title);
        

        if (strstr(line, "<message>"))
            sscanf(line, " <message>%[^<]</message>", temp_message);
    
     
        if (strstr(line, "</response>") && ok) {
            
            strcat(response, "Job Title: ");
            if (strlen(temp_job_title) > 0) 
                strcat(response, temp_job_title);
            else 
                strcat(response, "(Unknown)");
            
            strcat(response, "\n");
   
            strcat(response, "Response: ");
            strcat(response, temp_message);
            strcat(response, "\n\n");
        }
    }

    fclose(file);

    if (strlen(response) == 0) 
        strcpy(response, "No responses found.");
    
}


// functie pentru gestionare comenzi de la client
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;//pointer catre socket cand thread ul este creat, extragem valoarea
    free(arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (n <= 0) {
            printf("Client disconnected.\n");
            close(client_socket);
            return NULL;
        }

        char command[BUFFER_SIZE], param1[BUFFER_SIZE], param2[BUFFER_SIZE], param3[BUFFER_SIZE];
        memset(command, 0, sizeof(command));
        memset(param1, 0, sizeof(param1));
        memset(param2, 0, sizeof(param2));
        memset(param3, 0, sizeof(param3));

        sscanf(buffer, "%s %s %s %s", command, param1, param2, param3);

        if (strcmp(command, "LOGIN") == 0) {
            char status[BUFFER_SIZE] = {0};
            if (validate_user(param1, param2, status)) {
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "Login successful: %s", status);
                send(client_socket, response, strlen(response), 0);
            } else 
                send(client_socket, "Invalid credentials", 19, 0);
            
        } else if (strcmp(command, "REGISTER") == 0) {
            if (register_user(param1, param2, param3)) 
                send(client_socket, "Registration successful", 24, 0);
            else 
                send(client_socket, "User already exists", 20, 0);
            
        } else if (strcmp(command, "POST_JOB") == 0) {
            char titlu[BUFFER_SIZE], descriere[BUFFER_SIZE], salar[BUFFER_SIZE];
            sscanf(buffer + strlen("POST_JOB") + 1, "%[^|]|%[^|]|%[^\n]", titlu, descriere, salar);
            add_job(titlu, descriere, salar);
            send(client_socket, "Job added successfully!", 24, 0);
        } else if (strcmp(command, "SEARCH_JOBS") == 0) {
            char response[BUFFER_SIZE] = {0};
            get_all_jobs(response);
            send(client_socket, response, strlen(response), 0);
        } else if (strcmp(command, "POST_CV") == 0) {
            char titlu[BUFFER_SIZE], nume[BUFFER_SIZE], email[BUFFER_SIZE], experienta[BUFFER_SIZE];
            sscanf(buffer + strlen("POST_CV") + 1, "%[^|]|%[^|]|%[^|]|%[^\n]", titlu, nume, email, experienta);
            add_cv(titlu, nume, email, experienta);
            send(client_socket, "CV posted successfully!", 24, 0);
        }else if (strcmp(command, "CHECK_CV") == 0) {
            char titlu[BUFFER_SIZE] = {0};
            sscanf(buffer + strlen("CHECK_CV") + 1, "\"%[^\"]\"", titlu); // citire cu tot cu spatii
            char response[BUFFER_SIZE] = {0};
            get_cvs_for_job(titlu, response);
            send(client_socket, response, strlen(response), 0);
        }else if (strcmp(command, "ADD_TO_FAVOURITES") == 0) {
            char titlu[BUFFER_SIZE] = {0};
            sscanf(buffer + strlen("ADD_TO_FAVOURITES") + 1, "%s %[^\n]", param1, titlu);
            add_to_favourites(param1, titlu);
            send(client_socket, "Added to favourites!", 22, 0);
        } else if (strcmp(command, "VIEW_FAVOURITES") == 0) {
            char response[BUFFER_SIZE] = {0};
            get_favourites(param1, response);
            send(client_socket, response, strlen(response), 0);
        }
        else if (strcmp(command, "SEND_RESPONSE") == 0) {
            char job_title[BUFFER_SIZE], email[BUFFER_SIZE], message[BUFFER_SIZE];
            sscanf(buffer + strlen("SEND_RESPONSE") + 1, "%[^|]|%[^|]|%[^\n]", job_title, email, message);
            save_response(job_title, email, message);
            send(client_socket, "Response sent successfully!", 28, 0);
        } else if (strcmp(command, "VIEW_RESPONSES") == 0) {
            char response[BUFFER_SIZE] = {0};
            get_responses_for_employee(param1, response);
            send(client_socket, response, strlen(response), 0);
        }
        else 
            send(client_socket, "Unknown command", 15, 0);
        
    }

    return NULL;
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    initialize_user_db();
    initialize_jobs_db();
    initialize_cvs_db();
    initialize_responses_db();

    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

      // reutilizarea portului
    int on = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;//setam familie adrese IPV4
    server_addr.sin_addr.s_addr = INADDR_ANY;//sv asculta pe toate interfetele de retea available
    server_addr.sin_port = htons(PORT);//convertim portul in network byte order cu htons

   


    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {//legam socket-ul de adresa si port
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {//punem sv sa asculte daca vin clienti sa se con
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");

        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_socket_ptr) != 0) {//creeam cate un thread nou care executa functia handle_client
            perror("Thread creation failed");
            close(client_socket);
            free(client_socket_ptr);
        }

        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}