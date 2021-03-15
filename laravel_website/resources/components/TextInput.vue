<template>
  <div class="flex-column" :class="fullWidth">
    <label :for="name">{{ label }}</label>
    <input v-if="this.element === 'input'" autoComplete="off" :required="required" v-model="content" :maxlength="maxLength" :name="name" :type="type" @input="handleInput" class="input"/>
    <textarea v-if="this.element === 'textarea'" autoComplete="off" :required="required" v-model="content" :maxlength="maxLength" :name="name" :type="type" @input="handleInput" class="input textarea full-width"/>
  </div>
</template>

<script>
export default {
  name: "TextInput",
  props: {
    type: String,
    value: String,
    name: String,
    label: String,
    required: Boolean,
    limitWords: Number,
    maxLength: Number,
    element: {
      type: String,
      default: 'input'
    }
  },
  data() {
    return {
      content: this.value
    };
  },
  computed: {
    fullWidth() {
      return this.element === 'textarea' ? "full-width" : "";
    }
  },
  methods: {
    handleInput(event) {
      event.preventDefault();
      if (event.target.value.split(" ").length <= 300) {
        this.$emit("input", event.target.value);
      } else {
        this.content = event.target.value.split(" ").slice(0, 2).join(" ");
        this.$emit("input",event.target.value.split(" ").slice(0, 2).join(" ")
        );
      }
    }
  },
  watch: {
    value () {
      this.content = this.value
    }
  }
};
</script>

<style scoped>
.flex-column {
  display: flex;
  flex-direction: column;
  margin-right: 30px;
  width: 251px;
  margin-bottom: 20px;
  height: 100%;
}
.input {
  padding-left: 5px;
  padding-top: 5px;
  width: 100%;
  min-width: 200px;
  height: 40px;
  border: 1px solid #f2f2f2;
  box-shadow: inset 0px 0px 6px rgba(0, 0, 0, 0.04);
  border-radius: 4px;
  font-size: 1em;
}
.textarea {
  resize: none;
  height: 100%;
}
.full-width {
  width: 100%;
  min-height: 100px;
  margin-right: 0;
}
label {
  margin-bottom: 8px;
  font-size: 1.2em;
  line-height: 16px;
  color: #828282;
}

</style>
