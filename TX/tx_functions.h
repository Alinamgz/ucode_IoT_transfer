inline void mx_say_err(char *where_err);
void mx_do_my_keys(void);
void mx_do_msg(void);

inline void mx_do_sha (uint8_t* src, uint32_t src_len, uint8_t *rslt_buf, uint8_t chck);
void mx_recv_n_process_peer_key(void);
void mx_share_my_pub_key(void);
inline void mx_print_pkg(uint8_t *arr, uint32_t arr_len, uint8_t *prompt, uint32_t prompt_len);
inline int_fast16_t mx_chck_rf_termination_n_status(RF_EventMask terminationReason, uint32_t cmdStatus);
