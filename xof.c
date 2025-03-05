#include "xof.h"

int xof_init(xof_context *ctx)
{
	int ret = -1;

	if(ctx == NULL){
		goto err;
	}
	ctx->xof_finalized = 0;
	ret = _XOF_Init(&(ctx->ctx)); ERR(ret, err);

	ret = 0;
err:
	return ret;
}


int xof_update(xof_context *ctx, const uint8_t* data, size_t byte_len)
{
	int ret = -1;

	if(ctx == NULL){
		goto err;
	}
	ret = _XOF_Update(&(ctx->ctx), data, byte_len << 3); ERR(ret, err);

	ret = 0;
err:
	return ret;
}


int xof_squeeze(xof_context *ctx, uint8_t *out, uint32_t byte_len)
{
	int ret = -1;

	if(ctx == NULL){
		goto err;
	}
	/* NOTE: we transparently finalize the XOF to avoid inapropriate
	 * squeezing 
	 * */
	if(ctx->xof_finalized != 1){
		ret = _XOF_Final(&(ctx->ctx), NULL); ERR(ret, err);
		ctx->xof_finalized = 1;
	}
	ret = _XOF_Squeeze(&(ctx->ctx), out, byte_len << 3); ERR(ret, err);

	ret = 0;
err:
	return ret;
}
