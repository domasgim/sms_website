<template>
  <input :type="type" :value="value" />
</template>

<script>
export default {
  name: "Button",
  props: {
    value: String,
    type: String
  }
};
</script>

<style lang="scss" scoped>
input {
  margin: auto 0 20px auto;
  width: 115px;
  height: 40px;
  box-sizing: border-box;
  border: none;
  box-shadow: 0px 2px 10px rgba(0, 0, 0, 0.15);
  border-radius: 4px;
  font-weight: 500;
  font-size: 1em;
  line-height: 1em;
  text-align: center;
  text-transform: uppercase;
  background: #f5f7f9;
  color: #0054a6;
  cursor: pointer;
  &:hover {
    background: #ced0d3;
  }
}
</style>
