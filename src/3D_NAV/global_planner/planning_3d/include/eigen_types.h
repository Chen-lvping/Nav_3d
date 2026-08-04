#ifndef PLANNING_3D_EIGEN_TYPES_H
#define PLANNING_3D_EIGEN_TYPES_H

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <cstddef>

using Vec2i = Eigen::Vector2i;
using Vec3i = Eigen::Vector3i;
using Vec3b = Eigen::Matrix<char, 3, 1>;
using Vec2d = Eigen::Vector2d;
using Vec2f = Eigen::Vector2f;
using Vec3d = Eigen::Vector3d;
using Vec3f = Eigen::Vector3f;
using Vec4d = Eigen::Vector4d;
using Vec4f = Eigen::Vector4f;
using Vec5d = Eigen::Matrix<double, 5, 1>;
using Vec5f = Eigen::Matrix<float, 5, 1>;
using Vec6d = Eigen::Matrix<double, 6, 1>;
using Vec6f = Eigen::Matrix<float, 6, 1>;
using Vec9d = Eigen::Matrix<double, 9, 1>;
using Vec15d = Eigen::Matrix<double, 15, 1>;
using Vec18d = Eigen::Matrix<double, 18, 1>;

using Mat1d = Eigen::Matrix<double, 1, 1>;
using Mat2d = Eigen::Matrix<double, 2, 2>;
using Mat23d = Eigen::Matrix<double, 2, 3>;
using Mat32d = Eigen::Matrix<double, 3, 2>;
using Mat3d = Eigen::Matrix3d;
using Mat3f = Eigen::Matrix3f;
using Mat4d = Eigen::Matrix4d;
using Mat4f = Eigen::Matrix4f;
using Mat5d = Eigen::Matrix<double, 5, 5>;
using Mat5f = Eigen::Matrix<float, 5, 5>;
using Mat6d = Eigen::Matrix<double, 6, 6>;
using Mat6f = Eigen::Matrix<float, 6, 6>;
using Mat9d = Eigen::Matrix<double, 9, 9>;
using Mat96d = Eigen::Matrix<double, 9, 6>;
using Mat15d = Eigen::Matrix<double, 15, 15>;
using Mat18d = Eigen::Matrix<double, 18, 18>;
using VecXd = Eigen::Matrix<double, -1, 1>;
using MatXd = Eigen::Matrix<double, -1, -1>;
using MatX18d = Eigen::Matrix<double, -1, 18>;
using Quatd = Eigen::Quaterniond;
using Quatf = Eigen::Quaternionf;

inline const Mat3d Eye3d = Mat3d::Identity();
inline const Mat3f Eye3f = Mat3f::Identity();
inline const Vec3d Zero3d = Vec3d::Zero();
inline const Vec3f Zero3f = Vec3f::Zero();
using IdType = unsigned long;

namespace sad
{
template<int N>
struct map_range
{
  double x_min, x_max, y_min, y_max, z_min, z_max;
};

template<int N>
struct less_vec
{
  bool operator()(
    const Eigen::Matrix<int, N, 1> & lhs,
    const Eigen::Matrix<int, N, 1> & rhs) const;
};

template<int N>
struct hash_vec
{
  std::size_t operator()(const Eigen::Matrix<int, N, 1> & value) const;
};

template<>
inline bool less_vec<2>::operator()(const Vec2i & lhs, const Vec2i & rhs) const
{
  return lhs[0] < rhs[0] || (lhs[0] == rhs[0] && lhs[1] < rhs[1]);
}

template<>
inline bool less_vec<3>::operator()(const Vec3i & lhs, const Vec3i & rhs) const
{
  return lhs[0] < rhs[0] ||
         (lhs[0] == rhs[0] && lhs[1] < rhs[1]) ||
         (lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] < rhs[2]);
}

template<>
inline std::size_t hash_vec<2>::operator()(const Vec2i & value) const
{
  return static_cast<std::size_t>(
    ((value[0] * 73856093) ^ (value[1] * 471943)) % 10000000);
}

template<>
inline std::size_t hash_vec<3>::operator()(const Vec3i & value) const
{
  return static_cast<std::size_t>(
    ((value[0] * 73856093) ^ (value[1] * 471943) ^ (value[2] * 83492791)) %
    10000000);
}

inline constexpr auto less_vec2i = [](const Vec2i & lhs, const Vec2i & rhs) {
  return lhs[0] < rhs[0] || (lhs[0] == rhs[0] && lhs[1] < rhs[1]);
};
}  // namespace sad

#endif  // PLANNING_3D_EIGEN_TYPES_H
