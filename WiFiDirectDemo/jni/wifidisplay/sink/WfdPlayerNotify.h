#pragma once


namespace android {

struct WfdPlayerNotify : public RefBase {
	WfdPlayerNotify() {}
	virtual ~WfdPlayerNotify() {}

	virtual void notify(int msg, int ext1, int ext2) = 0;

	DISALLOW_EVIL_CONSTRUCTORS(WfdPlayerNotify);
};

}
