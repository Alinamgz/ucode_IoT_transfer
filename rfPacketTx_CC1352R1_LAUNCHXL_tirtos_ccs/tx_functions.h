inline void mx_say_err(char *where_err);
void mx_do_keys(void);
void mx_do_msg(void);

//inline void do_sha (uint8_t* src, uint32_t src_len, uint8_t *rslt_buf);
inline void do_sha (uint8_t* src, uint32_t src_len, uint8_t *rslt_buf, uint8_t chck);
void mx_handle_keypkg(uint8_t *packet, CryptoKey *peer_pub_key);
