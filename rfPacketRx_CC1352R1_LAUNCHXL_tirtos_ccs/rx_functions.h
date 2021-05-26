inline void mx_say_err(char *where_err);
void mx_do_keys(void);
inline void do_sha (uint8_t* src, uint32_t src_len, uint8_t *rslt_buf);
void mx_handle_keypkg(uint8_t *packet, CryptoKey *peer_pub_key);
