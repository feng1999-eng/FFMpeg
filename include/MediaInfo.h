#pragma once
class MediaInfo
{
public:
	static MediaInfo* instance() {
		static MediaInfo info;
		return &info;
	}
	int get_duration() {
		return duration;
	}
	void set_duration(int du) {
		duration = du;
	}
	~MediaInfo() = default;
private:
	MediaInfo()=default;
	int duration;
};

