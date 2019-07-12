#ifndef H_MAIN_H
#define H_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


int get_button_tries_sent();
int inc_button_tries_sent();
int reset_button_tries_sent();

int get_button_tries_error();
int inc_button_tries_error();
int reset_button_tries_error();

int get_current_data();
int set_current_data(int z);



#ifdef __cplusplus
}
#endif

#endif  /* H_MAIN_H */
