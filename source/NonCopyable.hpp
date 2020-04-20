#pragma once

class NonCopyable
{
private:
	NonCopyable(const NonCopyable&) {}
	NonCopyable& operator=(const NonCopyable&) {}

protected:
	NonCopyable() {}
	~NonCopyable() {}
};