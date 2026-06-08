// 실행 파일 위치 기반 경로 유틸리티를 선언합니다.
#pragma once
#include <string>

class Path {
public:
    static std::string GetExecutableDir();
    static std::string Resolve(const std::string& path);
};
