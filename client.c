#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Multiline_Output.H>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 12347
#define BUFFER_SIZE 1024

int client_socket;
Fl_Input *username_input, *password_input;
Fl_Choice *status_choice;
Fl_Multiline_Output *response_output;
Fl_Window *login_register_window, *employee_window, *employer_window;

//conectare la sv
int connect_to_server() {
    struct sockaddr_in server_addr;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection to server failed");
        close(client_socket);
        return -1;
    }

    return 0;
}

//functie pt trimitere comenzi catre sv
void send_command(const char *command, const char *param1, const char *param2, const char *param3, char *response) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "%s %s %s %s", command, param1, param2, param3);
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    int n = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (n <= 0) {
        strcpy(response, "Connection closed by server.");
        close(client_socket);
        exit(0);
    }

    strcpy(response, buffer);
}


void post_job_cb(Fl_Widget *, void *) {
    Fl_Window *post_job_window = new Fl_Window(400, 300, "Post Job");

    Fl_Input *job_title = new Fl_Input(100, 50, 200, 30, "Title:");
    Fl_Input *job_description = new Fl_Input(100, 100, 200, 30, "Description:");
    Fl_Input *job_salary = new Fl_Input(100, 150, 200, 30, "Salary:");

    Fl_Button *submit_button = new Fl_Button(150, 220, 100, 30, "Submit");

    submit_button->callback([](Fl_Widget *, void *data) {
        Fl_Input **inputs = (Fl_Input **)data;
        const char *titlu = inputs[0]->value();
        const char *descriere = inputs[1]->value();
        const char *salar = inputs[2]->value();

        if (strlen(titlu) == 0 || strlen(descriere) == 0 || strlen(salar) == 0) {
            response_output->value("All fields must be filled!");
            return;
        }

        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "POST_JOB %s|%s|%s", titlu, descriere, salar);
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        response_output->value(buffer);

    }, new Fl_Input *[3]{job_title, job_description, job_salary});

    post_job_window->end();
    post_job_window->show();
}

// buton searchjobs
void search_jobs_cb(Fl_Widget *, void *) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "SEARCH_JOBS");
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    Fl_Window *search_jobs_window = new Fl_Window(600, 400, "Search Jobs");
    Fl_Multiline_Output *output = new Fl_Multiline_Output(20, 20, 560, 360);
    output->value(buffer);

    search_jobs_window->end();
    search_jobs_window->show();
}

//post cv
void post_cv_cb(Fl_Widget *, void *) {
    
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "SEARCH_JOBS");
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    //fereastra
    Fl_Window *post_cv_window = new Fl_Window(400, 400, "Post CV");

    Fl_Choice *job_list = new Fl_Choice(100, 50, 200, 30, "Job Title:");
    Fl_Input *name = new Fl_Input(100, 100, 200, 30, "Name:");
    Fl_Input *email = new Fl_Input(100, 150, 200, 30, "Email:");
    Fl_Input *experience = new Fl_Input(100, 200, 200, 30, "Experience:");

    Fl_Button *submit_button = new Fl_Button(150, 300, 100, 30, "Submit");

    // extragem doar titlurile joburilor
    char *s = strtok(buffer, "\n");
    while (s != NULL) {
        if (strncmp(s, "Title:", 6) == 0) { 
            char *title = s + 7;         
            job_list->add(title);
        }
        s = strtok(NULL, "\n");
    }

    submit_button->callback([](Fl_Widget *, void *data) {
        Fl_Input **inputs = (Fl_Input **)data;
        Fl_Choice *job_list = (Fl_Choice *)inputs[0];
        const char *titlujob = job_list->text();
        const char *nume = inputs[1]->value();
        const char *email = inputs[2]->value();
        const char *experienta = inputs[3]->value();

        if (!titlujob || strlen(nume) == 0 || strlen(email) == 0 || strlen(experienta) == 0) {
            response_output->value("All fields must be filled!");
            return;
        }

        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "POST_CV %s|%s|%s|%s", titlujob, nume, email, experienta);
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        response_output->value(buffer);

    }, new Fl_Input *[4]{(Fl_Input *)job_list, name, email, experience});

    post_cv_window->end();
    post_cv_window->show();
}

void check_cv_cb(Fl_Widget *, void *) {
    Fl_Window *check_cv_window = new Fl_Window(400, 300, "Check CV");

    Fl_Input *job_title = new Fl_Input(100, 50, 200, 30, "Job Title:");
    Fl_Button *submit_button = new Fl_Button(150, 100, 100, 30, "Submit");

    submit_button->callback([](Fl_Widget *, void *data) {
    Fl_Input *job_title = (Fl_Input *)data;
    const char *job = job_title->value();

    if (strlen(job) == 0) {
        response_output->value("Job title cannot be empty!");
        return;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "CHECK_CV \"%s\"", job); 
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    Fl_Window *cv_results_window = new Fl_Window(600, 400, "CV Results");
    Fl_Multiline_Output *output = new Fl_Multiline_Output(20, 20, 560, 360);
    output->value(buffer);

    cv_results_window->end();
    cv_results_window->show();
}, job_title);

    check_cv_window->end();
    check_cv_window->show();
}

void login_cb(Fl_Widget *, void *) {
    const char *username = username_input->value();
    const char *parola = password_input->value();

    if (strlen(username) == 0 || strlen(parola) == 0) {
        response_output->value("Username or password cannot be empty.");
        return;
    }

    char rasp[BUFFER_SIZE] = {0};
    send_command("LOGIN", username, parola, "", rasp);

    if (strstr(rasp, "Login successful: employee")) {
        login_register_window->hide();
        employee_window->show();
    } 
    else if (strstr(rasp, "Login successful: employer"))
    {
        login_register_window->hide();
        employer_window->show();
    } 
    else 
        response_output->value(rasp);
    
}


void register_cb(Fl_Widget *, void *) {
    const char *username = username_input->value();
    const char *parola = password_input->value();
    const char *status = status_choice->text();

    if (strlen(username) == 0 || strlen(parola) == 0) {
        response_output->value("Username or password cannot be empty.");
        return;
    }

    char response[BUFFER_SIZE] = {0};
    send_command("REGISTER", username, parola, status, response);
    response_output->value(response);
}
void add_to_favourites_cb(Fl_Widget *, void *) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "SEARCH_JOBS");
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    Fl_Window *favourites_window = new Fl_Window(400, 400, "Add to Favourites");

    Fl_Choice *job_list = new Fl_Choice(100, 50, 200, 30, "Job Title:");
    Fl_Button *submit_button = new Fl_Button(150, 150, 100, 30, "Add");

    char *p = strtok(buffer, "\n");
    while (p != NULL) {
        if (strncmp(p, "Title:", 6) == 0) {
            char *title = p + 7;
            job_list->add(title);
        }
        p = strtok(NULL, "\n");
    }

    submit_button->callback([](Fl_Widget *, void *data) {
        Fl_Choice *job_list = (Fl_Choice *)data;
        const char *titlu = job_list->text();

        if (!titlu) {
            response_output->value("Please select a job title!");
            return;
        }

        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "ADD_TO_FAVOURITES %s %s", username_input->value(), titlu);
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        response_output->value(buffer);
    }, job_list);

    favourites_window->end();
    favourites_window->show();
}

void view_favourites_cb(Fl_Widget *, void *) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "VIEW_FAVOURITES %s", username_input->value());
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    Fl_Window *favourites_window = new Fl_Window(600, 400, "My Favourites");
    Fl_Multiline_Output *output = new Fl_Multiline_Output(20, 20, 560, 360);

    
    output->value(buffer);

    favourites_window->end();
    favourites_window->show();
}

void send_response_cb(Fl_Widget *, void *) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "SEARCH_JOBS");
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    Fl_Window *response_window = new Fl_Window(400, 400, "Send Response");

    Fl_Choice *job_list = new Fl_Choice(100, 50, 200, 30, "Job Title:");
    Fl_Input *cv_email = new Fl_Input(100, 100, 200, 30, "CV username:");
    Fl_Input *response_message = new Fl_Input(100, 150, 200, 30, "Response:");

    Fl_Button *submit_button = new Fl_Button(150, 250, 100, 30, "Submit");

    char *token = strtok(buffer, "\n");
    while (token != NULL) {
        if (strncmp(token, "Title:", 6) == 0) {
            char *title = token + 7;
            job_list->add(title);
        }
        token = strtok(NULL, "\n");
    }

    submit_button->callback([](Fl_Widget *, void *data) {
        Fl_Input **inputs = (Fl_Input **)data;
        Fl_Choice *job_list = (Fl_Choice *)inputs[0];
        const char *job_title = job_list->text();
        const char *id = inputs[1]->value();
        const char *message = inputs[2]->value();

        if (!job_title || strlen(id) == 0 || strlen(message) == 0) {
            response_output->value("All fields must be filled!");
            return;
        }

        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "SEND_RESPONSE %s|%s|%s", job_title, id, message);
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        response_output->value(buffer);

    }, new Fl_Input *[3]{(Fl_Input *)job_list, cv_email, response_message});

    response_window->end();
    response_window->show();
}

void view_responses_cb(Fl_Widget *, void *) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "VIEW_RESPONSES %s", username_input->value());

    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_received <= 0) {
        printf("Error receiving response from server.\n");
        response_output->value("Error receiving response from server.");
        return;
    }

    buffer[bytes_received] = '\0'; 
    printf("Response received: %s\n", buffer); 

    Fl_Window *responses_window = new Fl_Window(600, 400, "View Responses");
    Fl_Multiline_Output *output = new Fl_Multiline_Output(20, 20, 560, 360);
    output->value(buffer);

    responses_window->end();
    responses_window->show();
}


void apply_modern_style(Fl_Widget *widget, Fl_Color bg_color, Fl_Color text_color) {
    widget->color(bg_color);
    widget->labelcolor(text_color);
    widget->labelfont(FL_HELVETICA_BOLD);
    widget->labelsize(14);
}


void apply_button_style(Fl_Button *button) {
    button->box(FL_FLAT_BOX);
    button->color(FL_DARK3);
    button->labelcolor(FL_WHITE);
    button->labelfont(FL_HELVETICA_BOLD);
    button->labelsize(14);
}
void create_employee_window() {
    employee_window = new Fl_Window(600, 400, "Employee Dashboard");
    employee_window->color(FL_WHITE);

    Fl_Button *search_jobs = new Fl_Button(50, 50, 200, 30, "Search Jobs");
    search_jobs->callback(search_jobs_cb);
    apply_button_style(search_jobs);

    Fl_Button *post_cv = new Fl_Button(50, 100, 200, 30, "Post CV");
    post_cv->callback(post_cv_cb);
    apply_button_style(post_cv);

    Fl_Button *add_favourites = new Fl_Button(50, 150, 200, 30, "Add to Favourites");
    add_favourites->callback(add_to_favourites_cb);
    apply_button_style(add_favourites);

    Fl_Button *view_favourites = new Fl_Button(50, 200, 200, 30, "View Favourites");
    view_favourites->callback(view_favourites_cb);
    apply_button_style(view_favourites);

    Fl_Button *view_responses = new Fl_Button(50, 250, 200, 30, "View Responses");
    view_responses->callback(view_responses_cb);
    apply_button_style(view_responses);

    employee_window->end();
    employee_window->hide();
}


void create_employer_window() {
    employer_window = new Fl_Window(600, 400, "Employer Dashboard");
    employer_window->color(FL_WHITE);

    Fl_Button *post_job = new Fl_Button(50, 50, 200, 30, "Post Job");
    post_job->callback(post_job_cb);
    apply_button_style(post_job);

    Fl_Button *check_cv = new Fl_Button(50, 100, 200, 30, "Check CV");
    check_cv->callback(check_cv_cb);
    apply_button_style(check_cv);

    Fl_Button *send_response = new Fl_Button(50, 200, 200, 30, "Send Response");
    send_response->callback(send_response_cb);
    apply_button_style(send_response);

    employer_window->end();
    employer_window->hide();
}
int main() {
    if (connect_to_server() == -1) {
        printf("Failed to connect to server. Exiting...\n");
        return -1;
    }

    login_register_window = new Fl_Window(400, 350, "Main Page");
    login_register_window->color(FL_WHITE);

    Fl_Box *title_box = new Fl_Box(20, 20, 360, 40, "Welcome to RemoteJobBoard");
    title_box->labelfont(FL_HELVETICA_BOLD);
    title_box->labelsize(20);
    title_box->labelcolor(FL_DARK_BLUE);

    Fl_Box *username_label = new Fl_Box(50, 80, 80, 30, "Username:");
    username_label->labelsize(14);
    username_label->labelfont(FL_HELVETICA_BOLD);
    username_input = new Fl_Input(150, 80, 200, 30);

    Fl_Box *password_label = new Fl_Box(50, 130, 80, 30, "Password:");
    password_label->labelsize(14);
    password_label->labelfont(FL_HELVETICA_BOLD);
    password_input = new Fl_Input(150, 130, 200, 30);
    password_input->type(FL_SECRET_INPUT);

    Fl_Box *status_label = new Fl_Box(50, 180, 80, 30, "Status:");
    status_label->labelsize(14);
    status_label->labelfont(FL_HELVETICA_BOLD);
    status_choice = new Fl_Choice(150, 180, 200, 30);
    status_choice->add("employee|employer");
    status_choice->value(0);

    Fl_Button *login_button = new Fl_Button(80, 250, 100, 40, "Login");
    apply_button_style(login_button);
    login_button->callback(login_cb);

    Fl_Button *register_button = new Fl_Button(220, 250, 100, 40, "Register");
    apply_button_style(register_button);
    register_button->callback(register_cb);

    response_output = new Fl_Multiline_Output(50, 300, 300, 30);
    response_output->textfont(FL_HELVETICA);
    response_output->textsize(14);
    response_output->color(FL_BACKGROUND_COLOR);
    response_output->textcolor(FL_DARK_RED);

    login_register_window->end();

  
    create_employee_window();
    create_employer_window();

    login_register_window->show();

    return Fl::run();
}
